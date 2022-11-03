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
    seed(data::randomUniform()),
    _info(std::move(info)),
    _data(std::move(data)) {
    if (!password.empty()) {
        _passwordHash = crypto::blake2B(password);
    }
    _data.canSkip(true);
    _data.canOverwrite(true);
    _cleanTimerId = app().getLoop()->runEvery(chrono::seconds(60), [this] {
        if (_needClean) {
            clean();
        }
    });
}

Room::~Room() {
    app().getLoop()->invalidateTimer(_cleanTimerId);
    cancelStart();
    matchTryEnd(true);
    for (const auto &[playerId, wsConnRef]: _playerMap) {
        if (auto wsConnPtr = wsConnRef.lock()) {
            wsConnPtr->getContext<Player>()->reset();
            MessageJson(enum_integer(Action::RoomRemove)).to(wsConnPtr);
        } else {
            _needClean = true;
        }
    }
}

bool Room::empty(bool all) {
    if (all) {
        shared_lock<shared_mutex> lock(_playerMutex);
        return _playerMap.empty();
    } else {
        return countGamer() == 0;
    }
}

bool Room::full() { return countGamer() >= capacity; }

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
    Player::Type type{Player::Type::Spectator};
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        const auto node = _playerMap.extract(playerId);
        if (!node.empty()) {
            if (const auto &wsConnPtr = node.mapped().lock()) {
                type = wsConnPtr->getContext<Player>()->type.load();
                LOG_DEBUG << "Player " << playerId << " left room with type: " << enum_name(type);
            }
        }
    }
    if (empty(true)) {
        app().getPlugin<RoomManager>()->removeRoom(roomId);
    } else if (type == Player::Type::Gamer) {
        matchTryStart();
        matchTryEnd();
    }
}

int64_t Room::getFirstPlayerId() {
    for (const auto &[targetId, wsConnRef]: _playerMap) {
        if (auto wsConnPtr = wsConnRef.lock()) {
            return wsConnPtr->getContext<Player>()->playerId;
        }
    }
    return -1;
}

Json::Value Room::parse(bool details) {
    Json::Value result;
    result["roomId"] = roomId;
    result["capacity"] = capacity.load();
    result["count"]["Gamer"] = countGamer();
    result["count"]["Spectator"] = countSpectator();

    const auto tempState = state.load();
    if (tempState == State::Playing) {
        result["seed"] = seed.load();
    }
    result["state"] = string(enum_name(tempState));

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
                    _needClean = true;
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
                _needClean = true;
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

    _startTimerId = app().getLoop()->runAfter(3, [this]() {
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
                    _needClean = true;
                }
            }
        }
        Json::Value data;
        data["seed"] = seed.load();
        publish(MessageJson(enum_integer(Action::MatchStart)).setData(data));
    });
}

bool Room::cancelStart() {
    if (state == State::Ready) {
        app().getLoop()->invalidateTimer(_startTimerId.load());
        state = State::Standby;
        return true;
    }
    return false;
}

void Room::matchTryEnd(bool force) {
    if (state != State::Playing) {
        return;
    }

    if (!force) {
        if (countRemaining() > 1) {
            return;
        }
    }

    seed = data::randomUniform();
    state = State::Standby;
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (const auto &[playerId, wsConnRef]: _playerMap) {
            if (auto wsConnPtr = wsConnRef.lock()) {
                const auto &player = wsConnPtr->getContext<Player>();
                player->state = Player::State::Standby;
                player->clearHistory();
            } else {
                _needClean = true;
            }
        }
    }

    publish(MessageJson(enum_integer(Action::MatchEnd)));
}

uint64_t Room::countGamer() {
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            return player->type == Player::Type::Gamer;
        } else {
            _needClean = true;
        }
        return false;
    });
}

uint64_t Room::countRemaining() {
    unordered_set<uint64_t> groupSet;
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            if (player->state == Player::State::Playing) {
                if (player->group > 0) {
                    const auto [_, result] = groupSet.insert(player->group);
                    return result;
                }
                return true;
            }
        } else {
            _needClean = true;
        }
        return false;
    });
}

uint64_t Room::countSpectator() {
    shared_lock<shared_mutex> lock(_playerMutex);
    return count_if(_playerMap.begin(), _playerMap.end(), [&](const auto &item) {
        const auto &[playerId, wsConnRef] = item;
        if (const auto &wsConnPtr = wsConnRef.lock()) {
            const auto &player = wsConnPtr->template getContext<Player>();
            return player->type == Player::Type::Spectator;
        } else {
            _needClean = true;
        }
        return true;
    });
}

bool Room::isAllReady() {
    bool hasGamer = false;
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
            _needClean = true;
        }
        return true;
    }) && hasGamer;
}

void Room::clean() {
    {
        unique_lock<shared_mutex> lock(_playerMutex);
        erase_if(_playerMap, [](const auto &item) {
            const auto &[playerId, wsConnRef] = item;
            return wsConnRef.expired();
        });
    }
    if (empty(true)) {
        app().getPlugin<RoomManager>()->removeRoom(roomId);
    }
}
