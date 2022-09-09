//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomInfoGet.h>
#include <types/Action.h>
#include <types/JsonValue.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

RoomInfoGet::RoomInfoGet() : MessageHandlerBase(enum_integer(Action::RoomInfoGet)) {}

optional<string> RoomInfoGet::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    /// @note Reject if no room is specified and the player is not in the room
    if (!request.check("roomId", JsonValue::String) && !wsConnPtr->getContext<Player>()->getRoom()) {
        return i18n("roomNotFound");
    }
    return nullopt;
}

void RoomInfoGet::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        } else {
            room = wsConnPtr->getContext<Player>()->getRoom();
        }
        MessageJson(_action).setData(room->getInfo()).sendTo(wsConnPtr);
    }, _action, wsConnPtr);
}
