//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomInfoUpdate.h>
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

RoomInfoUpdate::RoomInfoUpdate() : MessageHandlerBase(enum_integer(Action::RoomInfoUpdate)) {}

optional<string> RoomInfoUpdate::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    /// @note Reject if the player is busy
    if (player->state != Player::State::Standby) {
        return i18n("notAvailable");
    }

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

    /// @note Reject if the data is not an array.
    if (!request.check("data", JsonValue::Array)) {
        return i18n("invalidArguments");
    }

    return nullopt;
}

void RoomInfoUpdate::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        } else {
            room = wsConnPtr->getContext<Player>()->getRoom();
        }
        room->publish(MessageJson(_action, MessageType::Server)
                              .setData(room->updateInfo(request["data"])));
    }, _action, wsConnPtr);
}
