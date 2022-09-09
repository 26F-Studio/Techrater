//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/PlayerManager.h>
#include <plugins/RoomManager.h>
#include <strategies/RoomDataGet.h>
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

RoomDataGet::RoomDataGet() : MessageHandlerBase(enum_integer(Action::RoomDataGet)) {}

optional<string> RoomDataGet::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto player = wsConnPtr->getContext<Player>();
    if (request.check("roomId", JsonValue::String) &&
        enum_cast<Permission>(player->playerInfo.getValueOfPermission()).value() != Permission::Admin) {
        return i18n("noPermission");
    } else if (!player->getRoom()) {
        return i18n("roomNotFound");
    }
    return nullopt;
}

void RoomDataGet::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    string roomId = player->getRoomId();
    if (request.check("roomId", JsonValue::String)) {
        roomId = request["roomId"].asString();
    }
    handleExceptions([&]() {
        app().getPlugin<RoomManager>()->roomDataGet(
                _action,
                wsConnPtr,
                roomId
        );
    }, _action, wsConnPtr);
}