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
        _playerMap(std::move(room._playerMap)),
        _connectionManager(room._connectionManager) {
    state = room.state.load();
    capacity = room.capacity.load();
    startTimerId = room.startTimerId.load();
}

Room::~Room() {
    cancelStart();
    matchTryEnd(true);
    for (const auto &[playerId, wsConnRef]: _playerMap) {
        if (auto wsConnPtr = wsConnRef.lock()) {
            wsConnPtr->getContext<Player>()->reset();
            MessageJson(enum_integer(Action::RoomRemove)).to(wsConnPtr);
        } else {
            // TODO: Clear invalid connections
        }
    }
}

bool Room::empty(bool all) const {
    if (all) {
        shared_lock<shared_mutex> lock(_playerMutex);
        return _playerMap.empty();
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

void Room::subscribe(const WebSocketConnectionPtr &wsConnPtr) {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        _playerMap.emplace(wsConnPtr->getContext<Player>()->playerId, wsConnPtr);
    }
    cancelStart();
}

void Room::unsubscribe(int64_t playerId) {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        _playerMap.erase(playerId);
    }
    if (empty(true)) {
        app().getPlugin<RoomManager>()->removeRoom(roomId);
    } else {
        matchTryStart();
        matchTryEnd();
    }
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
            for (const auto &[playerId, wsConnRef]: _playerMap) {
                if (auto wsConnPtr = wsConnRef.lock()) {
                    result["players"].append(wsConnPtr->getContext<Player>()->info());
                } else {
                    // TODO: Clear invalid connections
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
    for (const auto &[playerId, wsConnRef]: _playerMap) {
        if (excludedId != playerId) {
            if (auto wsConnPtr = wsConnRef.lock()) {
                message.to(wsConnPtr);
            } else {
                // TODO: Clear invalid connections
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
            for (const auto &[playerId, wsConnRef]: _playerMap) {
                if (auto wsConnPtr = wsConnRef.lock()) {
                    const auto &player = wsConnPtr->getContext<Player>();
                    if (player->type == Player::Type::Gamer) {
                        player->state = Player::State::Playing;
                    }
                } else {
                    // TODO: Clear invalid connections
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

void Room::matchTryEnd(bool force) {
    if (state != State::Playing ||
        (!force && countPlaying() > 1)) {
        return;
    }

    state = State::Standby;
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (const auto &[playerId, wsConnRef]: _playerMap) {
            if (auto wsConnPtr = wsConnRef.lock()) {
                wsConnPtr->getContext<Player>()->state = Player::State::Standby;
            } else {
                // TODO: Clear invalid connections
            }
        }
    }

    publish(MessageJson(enum_integer(Action::MatchEnd)));
}

uint64_t Room::countGamer() const {
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            return player->type == Player::Type::Gamer;
        } else {
            // TODO: Clear invalid connections
        }
        return true;
    });
}

uint64_t Room::countSpectator() const {
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            return player->type == Player::Type::Spectator;
        } else {
            // TODO: Clear invalid connections
        }
        return true;
    });
}

uint64_t Room::countPlaying() const {
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            return player->type == Player::Type::Gamer && player->state == Player::State::Playing;
        } else {
            // TODO: Clear invalid connections
        }
        return true;
    });
}

bool Room::isAllReady() const {
    bool hasExpired = false, hasGamer = false;
    shared_lock<shared_mutex> lock(_playerMutex);
    return all_of(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            if (player->type == Player::Type::Gamer) {
                if (player->state != Player::State::Ready) {
                    return false;
                } else if (!hasGamer) {
                    hasGamer = true;
                }
            }
        } else {
            // TODO: Clear invalid connections
        }
        return true;
    }) && hasGamer;
}

void Room::refresh() {
    unique_lock<shared_mutex> lock(_playerMutex);
    erase_if(_playerMap, [](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        return wsConnRef.expired();
    });
}