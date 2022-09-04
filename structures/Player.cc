//
// Created by g29624 on 2022/8/30.
//

#include <structures/Player.h>

using namespace drogon;
using namespace std;
using namespace techmino::structures;

Player::Player(
        int64_t userId,
        Role role,
        State state,
        Type type
) : PlayerBase(userId),
    role(role),
    state(state),
    type(type) {}

Player::Player(Player &&player) noexcept:
        PlayerBase(player.userId),
        _customState(std::move(player._customState)),
        _config(std::move(player._config)) {
    group = player.group.load();
    role = player.role.load();
    state = player.state.load();
    type = player.type.load();
}

shared_ptr<Room> Player::getRoom() const {
    return nullptr;
}

void Player::setRoom(std::shared_ptr<Room> room) {

}

string Player::getCustomState() const {
    return std::string();
}

void Player::setCustomState(string &&customState) {

}

string Player::getConfig() const {
    return std::string();
}

void Player::setConfig(string &&config) {

}

Json::Value Player::info() const {
    return Json::Value();
}

void Player::reset() {

}
