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
        _playerSet(std::move(room._playerSet)),
        _connectionManager(room._connectionManager) {
    state = room.state.load();
    capacity = room.capacity.load();
    startTimerId = room.startTimerId.load();
}

Room::~Room() {
    cancelStart();
    matchEnd(true);
    for (const auto playerId: _playerSet) {
        try {
            const auto &wsConnPtr = _connectionManager->getConnPtr(playerId);
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
        return _playerSet.empty();
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

void Room::subscribe(int64_t playerId) {
    unique_lock<shared_mutex> lock(_playerMutex);
    _playerSet.insert(playerId);
}

void Room::unsubscribe(int64_t playerId) {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        _playerSet.erase(playerId);
    }
    if (empty(true)) {
        app().getPlugin<RoomManager>()->removeRoom(roomId);
    } else if (empty()) {
        matchEnd();
    }
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

uint64_t Room::countStandby() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        try {
            const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
            if (player->type == Player::Type::Gamer &&
                player->state == Player::State::Standby) {
                counter++;
            }
        } catch (const structures::MessageException &) {
            LOG_WARN << "Player " << playerId << " not found";
        }
    }
    return counter;
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

void Room::matchStart(bool force) {
    if (state != State::Standby ||
        (!force && countStandby() > 0)) {
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

bool Room::cancelStart() {
    if (state == State::Ready) {
        app().getLoop()->invalidateTimer(startTimerId.load());
        state = State::Standby;
        return true;
    }
    return false;
}

void Room::matchEnd(bool force) {
    if (state != State::Playing ||
        (!force && countPlaying() > 0)) {
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
