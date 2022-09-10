//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomKick.h>
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

RoomKick::RoomKick() : MessageHandlerBase(enum_integer(Action::RoomKick)) {}

optional<string> RoomKick::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();

    /// @note Reject if no playerId is specified.
    if (!request.check("playerId", JsonValue::Int64) ||
        request["playerId"].asInt64() == player->playerId) {
        return i18n("invalidArguments");
    }

    /// @note Check if accessing current room.
    if (request.check("roomId", JsonValue::String)) {
        /// @note Reject if the player is not a global admin.
        if (enum_cast<Permission>(player->playerInfo.getValueOfPermission()).value() != Permission::Admin) {
            return i18n("noPermission");
        }
    } else if (player->getRoom()) {
        /// @note Reject if the player's role is lower than the target.
        if (player->role <
            app().getPlugin<ConnectionManager>()->getConnPtr(request.ref().asInt64())->getContext<Player>()->role) {
            return i18n("noPermission");
        }
    } else {
        /// @note Reject if no room is specified.
        return i18n("roomNotFound");
    }

    return nullopt;
}

void RoomKick::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        } else {
            room = wsConnPtr->getContext<Player>()->getRoom();
        }

        room;

        app().getPlugin<RoomManager>()->roomKick(
                _action,
                app().getPlugin<ConnectionManager>()->getConnPtr(request.ref().asInt64())
        );
    }, _action, wsConnPtr);
}
