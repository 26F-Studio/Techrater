//
// Created by g29624 on 2022/8/30.
//

#include <magic_enum.hpp>
#include <structures/Player.h>
#include <types/Action.h>

using namespace drogon;
using namespace drogon_model;
using namespace magic_enum;
using namespace std;
using namespace techmino::structures;
using namespace techmino::types;

Player::Player(
        int64_t playerId,
        Role role,
        State state,
        Type type
) : PlayerBase(playerId),
    playerInfo(orm::Mapper<techrater::Player>(app().getDbClient()).findByPrimaryKey(playerId)),
    role(role),
    state(state),
    type(type) {}

Player::Player(Player &&player) noexcept:
        PlayerBase(player.playerId),
        _customState(std::move(player._customState)),
        _config(std::move(player._config)) {
    group = player.group.load();
    role = player.role.load();
    state = player.state.load();
    type = player.type.load();
}

Player::~Player() {
    // TODO: Check if this would cause problems
    if (_room) {
        _room->unsubscribe(playerId);
        Json::Value data;
        data["playerId"] = playerId;
        _room->publish(MessageJson(enum_integer(Action::RoomLeave)).setData(std::move(data)), playerId);
    }
}

shared_ptr<Room> Player::getRoom() const {
    shared_lock<shared_mutex> lock(_dataMutex);
    return _room;
}

void Player::setRoom(shared_ptr<Room> room) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _room = std::move(room);
}

void Player::setCustomState(string &&customState) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _customState = std::move(customState);
}

void Player::setConfig(string &&config) {
    unique_lock<shared_mutex> lock(_dataMutex);
    _config = std::move(config);
}

void Player::appendHistory(const std::string &history) {
    unique_lock<shared_mutex> lock(_historyMutex);
    _history += history;
}

string Player::history() const {
    shared_lock<shared_mutex> lock(_historyMutex);
    return _history;
}

Json::Value Player::info() const {
    Json::Value info;
    info["playerId"] = playerId;
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

void Player::reset() {
    group = 0;
    role = Role::Normal;
    state = State::Standby;
    type = Type::Spectator;

    {
        unique_lock<shared_mutex> lock(_dataMutex);
        _room = nullptr;
        _customState.clear();
    }
    {
        unique_lock<shared_mutex> lock(_historyMutex);
        _history.clear();
    }
}
