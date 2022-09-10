//
// Created by g29624 on 2022/8/30.
//

#include <magic_enum.hpp>
#include <structures/Player.h>

using namespace drogon;
using namespace drogon_model;
using namespace magic_enum;
using namespace std;
using namespace techmino::structures;

Player::Player(
        int64_t userId,
        Role role,
        State state,
        Type type
) : PlayerBase(userId),
    playerInfo(orm::Mapper<techrater::Player>(app().getDbClient()).findByPrimaryKey(userId)),
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

shared_ptr<Room> Player::getRoom() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    return _room;
}

void Player::setRoom(shared_ptr<Room> room) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _room = std::move(room);
}

string Player::getCustomState() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    return _customState;
}

void Player::setCustomState(string &&customState) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _customState = std::move(customState);
}

string Player::getConfig() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    return _config;
}

void Player::setConfig(string &&config) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _config = std::move(config);
}

Json::Value Player::info() const {
    Json::Value info;
    info["playerId"] = playerId;
    info["group"] = group.load();
    info["role"] = string(enum_name(role.load()));
    info["type"] = string(enum_name(type.load()));

    shared_lock<shared_mutex> lock(_sharedMutex);
    if (state == State::Playing) {
        info["state"] = _customState;
    } else {
        info["state"] = string(enum_name(state.load()));
    }
    info["config"] = _config;
    return info;
}

void Player::reset() {
    _room->unsubscribe(playerId);
    group = 0;
    role = Role::Normal;
    state = State::Standby;
    type = Type::Spectator;

    unique_lock<shared_mutex> lock(_sharedMutex);
    _room = nullptr;
    _customState.clear();
}

Player::~Player() {
    _room->unsubscribe(playerId);
}
