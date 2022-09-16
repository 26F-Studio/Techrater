//
// Created by particleg on 2021/10/8.
//

#include <magic_enum.hpp>
#include <structures/Player.h>
#include <structures/Room.h>
#include <types/Action.h>
#include <types/MessageType.h>
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
    _passwordHash(crypto::blake2B(password)),
    _info(std::move(info)),
    _data(std::move(data)),
    _connectionManager(app().getPlugin<ConnectionManager>()) {
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

bool Room::checkPassword(const string &password) const {
    shared_lock<shared_mutex> lock(_dataMutex);
    return crypto::blake2B(password) == _passwordHash;
}

void Room::updatePassword(const string &password) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _passwordHash = crypto::blake2B(password);
}

void Room::subscribe(int64_t playerId) {
    unique_lock<shared_mutex> lock(_playerMutex);
    _playerSet.insert(playerId);
}

void Room::unsubscribe(int64_t playerId) {
    unique_lock<shared_mutex> lock(_playerMutex);
    _playerSet.erase(playerId);
}

uint64_t Room::countGamer() const {
    uint64_t counter{};
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (const auto playerId: _playerSet) {
            const auto &player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
            if (player->type == Player::Type::Gamer) {
                counter++;
            }
        }
    }
    return counter;
}

uint64_t Room::countPlaying() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
        if (player->type == Player::Type::Gamer &&
            player->state == Player::State::Playing) {
            counter++;
        }
    }
    return counter;
}

uint64_t Room::countSpectator() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
        if (player->type == Player::Type::Spectator) {
            counter++;
        }
    }
    return counter;
}

uint64_t Room::countStandby() const {
    uint64_t counter{};
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        const auto player = _connectionManager->getConnPtr(playerId)->getContext<Player>();
        if (player->type == Player::Type::Gamer &&
            player->state == Player::State::Standby) {
            counter++;
        }
    }
    return counter;
}

bool Room::empty() const { return countGamer() == 0; }

bool Room::full() const { return countGamer() >= capacity; }

Json::Value Room::parse(bool details) const {
    Json::Value result;
    result["roomId"] = roomId;
    result["capacity"] = capacity.load();
    result["state"] = string(enum_name(state.load()));
    result["count"]["gamer"] = countGamer();
    result["count"]["spectator"] = countSpectator();

    {
        shared_lock<shared_mutex> dataLock(_dataMutex);
        result["info"] = _info.copy();
        if (details) {
            result["data"] = _data.copy();
        }
    }

    if (details) {
        result["players"] = Json::arrayValue;
        shared_lock<shared_mutex> playerLock(_playerMutex);
        for (const auto playerId: _playerSet) {
            result["players"].append(
                    _connectionManager->getConnPtr(playerId)->getContext<Player>()->info()
            );
        }
    }

    return result;
}

void Room::publish(MessageJson &message, int64_t excludedId) {
    shared_lock<shared_mutex> lock(_playerMutex);
    for (const auto playerId: _playerSet) {
        if (excludedId != playerId) {
            message.sendTo(_connectionManager->getConnPtr(playerId));
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

void Room::startGame(bool force) {
    if (state != State::Standby ||
        (!force && countStandby() > 0)) {
        return;
    }

    state = State::Ready;
    publish(MessageJson(enum_integer(Action::GameReady)).setMessageType(MessageType::Server));

    startTimerId = app().getLoop()->runAfter(3, [this]() {
        state = State::Playing;
        publish(MessageJson(enum_integer(Action::GameStart)).setMessageType(MessageType::Server));
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

void Room::endGame(bool force) {
    if (state != State::Playing ||
        (!force && countPlaying() > 0)) {
        return;
    }

    state = State::Standby;
    {
        shared_lock<shared_mutex> lock(_playerMutex);
        for (auto &playerId: _playerSet) {
            _connectionManager->getConnPtr(playerId)->getContext<Player>()->state = Player::State::Standby;
        }
    }

    publish(MessageJson(enum_integer(Action::GameEnd)).setMessageType(MessageType::Server));
}

Room::~Room() {
    cancelStart();
    endGame(true);
    for (const auto playerId: _playerSet) {
        const auto &wsConnPtr = _connectionManager->getConnPtr(playerId);
        wsConnPtr->getContext<Player>()->reset();

        MessageJson(enum_integer(Action::RoomRemove))
                .setMessageType(MessageType::Server)
                .sendTo(wsConnPtr);
    }
}