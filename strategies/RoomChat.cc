//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomChat.h>
#include <types/Action.h>
#include <types/JsonValue.h>
#include <types/Permission.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

RoomChat::RoomChat() : MessageHandlerBase(enum_integer(Action::RoomCreate)) {}

optional<string> RoomChat::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();

    /// @note Check if accessing current room.
    if (request.check("roomId", JsonValue::String)) {
        /// @note Reject if the player is not a global admin.
        if (enum_cast<Permission>(player->playerInfo.getValueOfPermission()).value() != Permission::Admin) {
            return i18n("noPermission");
        }
    } else if (player->getRoom()) {
        /// @note Reject if the player is not a room admin.
        if (player->role < Player::Role::Admin) {
            return i18n("noPermission");
        }
    } else {
        /// @note Reject if no room is specified.
        return i18n("roomNotFound");
    }

    /// @note Reject if the message is not an string.
    if (!request.check("message", JsonValue::String)) {
        return i18n("invalidArguments");
    }

    request.trim("password", JsonValue::String);

    return nullopt;
}

void RoomChat::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        } else {
            room = player->getRoom();
        }
        Json::Value data;
        data["playerId"] = player->playerId;
        data["config"] = request["message"].asString();

        room->publish(MessageJson(_action).setData(data));
        room->appendChat(std::move(data));
    }, _action, wsConnPtr);
}