//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomDataUpdate.h>
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

RoomDataUpdate::RoomDataUpdate() : MessageHandlerBase(enum_integer(Action::RoomDataUpdate)) {}

optional<string> RoomDataUpdate::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();

    /// @note Reject if the player is busy
    if (player->state != Player::State::Standby) {
        return i18n("notAvailable");
    }

    /// @note Check if accessing current room.
    if (player->getRoom()) {
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

void RoomDataUpdate::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
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
        data["data"] = room->updateData(request["data"]);

        room->publish(MessageJson(_action).setData(std::move(data)));
    }, _action, wsConnPtr);
}