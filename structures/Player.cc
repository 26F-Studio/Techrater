//
// Created by g29624 on 2022/8/30.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <structures/Player.h>
#include <types/Action.h>

using namespace drogon;
using namespace drogon_model;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::structures;
using namespace techmino::types;

Player::Player(int64_t playerId) :
        PlayerBase(playerId),
        playerInfo(orm::Mapper<techrater::Player>(app().getDbClient()).findByPrimaryKey(playerId)) {}

Player::Player(Player &&player) noexcept: PlayerBase(player.playerId) {
    _room.swap(player._room);
}

Player::~Player() {
    // TODO: Check if this would cause problems
    if (_room) {
        _room->unsubscribe(playerId);
        Json::Value data;
        data["playerId"] = playerId;
        LOG_DEBUG << "Player:" << playerId << " publishing leaving message...";
        _room->publish(MessageJson(enum_integer(Action::RoomLeave)).setData(std::move(data)), playerId);
    }
}

shared_ptr<Room> Player::getRoom() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    return _room;
}

void Player::setRoom(shared_ptr<Room> room) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _room = std::move(room);
}

void Player::reset() {
    unique_lock<shared_mutex> lock(_sharedMutex);
    _room = nullptr;
}
