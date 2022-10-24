//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomKick.h>
#include <types/Action.h>
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
    const auto &roomManager = app().getPlugin<RoomManager>();
    const auto &player = wsConnPtr->getContext<Player>();
    const auto &targetConnPtr = app().getPlugin<ConnectionManager>()->getConnPtr(request["playerId"].asInt64());
    const auto &target = targetConnPtr->getContext<Player>();

    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = roomManager->getRoom(request["roomId"].asString());
        } else {
            room = wsConnPtr->getContext<Player>()->getRoom();
        }

        Json::Value data;
        data["executorId"] = player->playerId;
        data["playerId"] = target->playerId;
        auto message = MessageJson(_action).setData(std::move(data));

        room->unsubscribe(target->playerId);
        room->publish(message, target->playerId);

        target->reset();
        message.to(targetConnPtr);
    }, _action, wsConnPtr);
}
