//
// Created by particleg on 2021/10/8.
//

#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <structures/Player.h>
#include <structures/Room.h>
#include <types/Action.h>
#include <types/ErrorNumber.h>
#include <utils/crypto.h>


using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;
using namespace trantor;

using Member = Room::Member;

Member::Member(
        int64_t playerId,
        Member::Role role,
        Member::State state,
        Member::Type type
) : PlayerBase(playerId),
    role(role),
    state(state),
    type(type) {}

Member::Member(Member &&member) noexcept:
        PlayerBase(member.playerId),
        _config(std::move(member._config)),
        _customState(std::move(member._customState)),
        _history(std::move(member._history)) {
    group = member.group.load();
    role = member.role.load();
    state = member.state.load();
    type = member.type.load();
}

void Member::setConfig(string &&config) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _config = std::move(config);
}

bool Member::hasConfig() const {
    shared_lock<shared_mutex> lock(_dataMutex);
    return !_config.empty();
}

void Member::setCustomState(string &&customState) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _customState = std::move(customState);
}

void Member::appendHistory(const string &history) {
    unique_lock<shared_mutex> lock(_historyMutex);
    _history += history;
}

string Member::history() const {
    shared_lock<shared_mutex> lock(_historyMutex);
    return _history;
}

Json::Value Member::info() const {
    Json::Value info;
    info["group"] = group.load();
    info["role"] = string(enum_name(role.load()));
    info["type"] = string(enum_name(type.load()));
    {
        shared_lock<shared_mutex> lock(_dataMutex);
        if (state == State::Playing && !_customState.empty()) {
            info["state"] = _customState;
        } else {
            info["state"] = string(enum_name(state.load()));
        }
        info["config"] = _config;
    }
    {
        shared_lock<shared_mutex> lock(_historyMutex);
        info["history"] = _history;
    }
    return info;
}

void Room::Member::reset() {
    group = 0;
    role = Role::Normal;
    state = State::Standby;
    type = Type::Spectator;
    {
        unique_lock<shared_mutex> lock(_dataMutex);
        _customState.clear();
    }
    {
        unique_lock<shared_mutex> lock(_historyMutex);
        _history.clear();
    }
}

Room::Room(
        uint64_t capacity,
        const string &password,
        Json::Value info,
        Json::Value data
) : capacity(capacity),
    _info(std::move(info)),
    _data(std::move(data)),
    _connectionManager(app().getPlugin<ConnectionManager>()) {
    if (!password.empty()) {
        _passwordHash = crypto::blake2B(password);
    }
    _data.canSkip(true);
    _data.canOverwrite(true);
}

Room::Room(Room &&room) noexcept:
        roomId(room.roomId),
        _passwordHash(std::move(room._passwordHash)),
        _info(std::move(room._info.ref())),
        _data(std::move(room._data.ref())),
        _memberMap(std::move(room._memberMap)),
        _connectionManager(room._connectionManager) {
    state = room.state.load();
    capacity = room.capacity.load();
    startTimerId = room.startTimerId.load();
}

Room::~Room() {
    matchCancelStart();
    matchTryEnd(true);
    for (const auto &[playerId, _]: _memberMap) {
        try {
            const auto wsConnPtr = _connectionManager->getConnPtr(playerId);
            wsConnPtr->getContext<Player>()->reset();
            MessageJson(enum_integer(Action::RoomRemove)).to(wsConnPtr);
        } catch (const structures::MessageException &) {
            LOG_WARN << "Player " << playerId << " not found";
        }
    }
}

bool Room::empty(bool all) const {
    if (all) {
        shared_lock<shared_mutex> lock(_playerMutex);
        return _memberMap.empty();
    } else {
        return countGamer() == 0;
    }
}

bool Room::full() const { return countGamer() >= capacity; }

bool Room::checkPassword(const string &password) const {
    shared_lock<shared_mutex> lock(_dataMutex);
    if (_passwordHash.empty() && password.empty()) {
        return true;
    }
    return crypto::blake2B(password) == _passwordHash;
}

void Room::updatePassword(const string &password) {
    unique_lock<shared_mutex> lock(_dataMutex);
    if (password.empty()) {
        _passwordHash.clear();
    } else {
        _passwordHash = crypto::blake2B(password);
    }
}

void Room::subscribe(Member &&member) {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        _memberMap.emplace(member.playerId, std::move(member));
    }
    matchCancelStart();
}

void Room::unsubscribe(int64_t playerId) {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        _playerSet.erase(playerId);
    }
    if (empty(true)) {
        app().getPlugin<RoomManager>()->removeRoom(roomId);
    } else {
        matchTryStart();
        matchTryEnd();
    }
    // TODO: Check if there's any Admins in the room
}

Json::Value Room::parse(bool details) const {
    Json::Value result;
    result["roomId"] = roomId;
    result["capacity"] = capacity.load();
    result["state"] = string(enum_name(state.load()));
    result["count"]["Gamer"] = countGamer();
    result["count"]["Spectator"] = countSpectator();

    {
        shared_lock<shared_mutex> dataLock(_dataMutex);
        result["private"] = !_passwordHash.empty();
        result["info"] = _info.copy();
        if (details) {
            result["data"] = _data.copy();
        }
    }

    if (details) {
        result["players"] = Json::arrayValue;
        result["chats"] = Json::arrayValue;
        {
            shared_lock<shared_mutex> playerLock(_playerMutex);
            for (const auto playerId: _playerSet) {
                try {
                    result["players"].append(
                            _connectionManager->getConnPtr(playerId)->getContext<Player>()->info()
                    );
                } catch (const structures::MessageException &) {
                    LOG_WARN << "Player " << playerId << " not found";
                }
            }
        }

        {
            shared_lock<shared_mutex> lock(_chatMutex);
            for (const auto &chat: _chatList) {
                result["chats"].append(chat);
            }
        }
    }

    return result;
}

void Room::publish(const MessageJson &message, int64_t excludedId) {
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        if (excludedId != playerId) {
            try {
                message.to(_connectionManager->getConnPtr(playerId));
            } catch (const structures::MessageException &) {
                LOG_WARN << "Player " << playerId << " not found";
            }
        }
    }
}

Json::Value Room::getData() const {
    shared_lock<shared_mutex> lock(_dataMutex);
    return _data.copy();
}

Json::Value Room::updateData(const Json::Value &data) {
    shared_lock<shared_mutex> lock(_dataMutex);
    for (const auto &item: data) {
        _data.modifyByPath(item["path"].asString(), item["value"]);
    }
    return _data.copy();
}

Json::Value Room::getInfo() const {
    shared_lock<shared_mutex> lock(_dataMutex);
    return _info.copy();
}

Json::Value Room::updateInfo(const Json::Value &data) {
    shared_lock<shared_mutex> lock(_dataMutex);
    for (const auto &item: data) {
        _info.modifyByPath(item["path"].asString(), item["value"]);
    }
    return _info.copy();
}

void Room::appendChat(Json::Value &&chat) {
    unique_lock<shared_mutex> lock(_chatMutex);
    _chatList.push_back(std::move(chat));
}

void Room::matchTryStart(bool force) {
    if (!(state == State::Standby && (force || isAllReady()))) {
        return;
    }

    state = State::Ready;
    publish(MessageJson(enum_integer(Action::MatchReady)));

    startTimerId = app().getLoop()->runAfter(3, [this]() {
        state = State::Playing;
        {
            shared_lock<shared_mutex> lock(_playerMutex);
            for (const auto playerId: _playerSet) {
                try {
                    const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
                    if (player->type == Player::Type::Gamer) {
                        player->state = Player::State::Playing;
                    }
                } catch (const structures::MessageException &) {
                    LOG_WARN << "Player " << playerId << " not found";
                }
            }
        }
        publish(MessageJson(enum_integer(Action::MatchStart)).setData(data::randomUniform()));
    });
}

bool Room::matchCancelStart() {
    if (state == State::Ready) {
        app().getLoop()->invalidateTimer(startTimerId.load());
        state = State::Standby;
        return true;
    }
    return false;
}

void Room::matchTryEnd(bool force) {
    if (state != State::Playing ||
        (!force && countPlaying() > 1)) {
        return;
    }

    state = State::Standby;
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (auto &playerId: _playerSet) {
            try {
                _connectionManager->getConnPtr(playerId)->getContext<Player>()->state = Player::State::Standby;
            } catch (const structures::MessageException &) {
                LOG_WARN << "Player " << playerId << " not found";
            }
        }
    }

    publish(MessageJson(enum_integer(Action::MatchEnd)));
}

uint64_t Room::countGamer() const {
    uint64_t counter{};
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (const auto playerId: _playerSet) {
            try {
                const auto &player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
                if (player->type == Player::Type::Gamer) {
                    counter++;
                }
            } catch (const structures::MessageException &) {
                LOG_WARN << "Player " << playerId << " not found";
            }
        }
    }
    return counter;
}

uint64_t Room::countSpectator() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        try {
            const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
            if (player->type == Player::Type::Spectator) {
                counter++;
            }
        } catch (const structures::MessageException &) {
            LOG_WARN << "Player " << playerId << " not found";
        }
    }
    return counter;
}

uint64_t Room::countPlaying() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        try {
            const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
            if (player->type == Player::Type::Gamer &&
                player->state == Player::State::Playing) {
                counter++;
            }
        } catch (const structures::MessageException &) {
            LOG_WARN << "Player " << playerId << " not found";
        }
    }
    return counter;
}

bool Room::isAllReady() const {
    bool hasGamer = false;
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        try {
            const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
            if (player->type == Player::Type::Gamer) {
                if (player->state != Player::State::Ready) {
                    return false;
                } else if (!hasGamer) {
                    hasGamer = true;
                }
            }
        } catch (const structures::MessageException &) {
            LOG_WARN << "Player " << playerId << " not found";
        }
    }
    return hasGamer;
}
