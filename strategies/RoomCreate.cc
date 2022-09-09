//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomCreate.h>
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

RoomCreate::RoomCreate() : MessageHandlerBase(enum_integer(Action::RoomCreate)) {}

optional<string> RoomCreate::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    if (wsConnPtr->getContext<Player>()->getRoom()) {
        return i18n("notAvailable");
    }

    if (!request.check("capacity", JsonValue::Uint64) ||
        !request.check("info", JsonValue::Object) ||
        !request.check("data", JsonValue::Object) ||
        request["capacity"].asUInt64() == 0) {
        return i18n("invalidArguments");
    }

    request.trim("password", JsonValue::String);
    return nullopt;
}

void RoomCreate::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    handleExceptions([&]() {
        app().getPlugin<RoomManager>()->roomCreate(
                _action,
                wsConnPtr,
                request["capacity"].asUInt64(),
                request["password"].asString(),
                request["info"],
                request["data"]
        );
    }, _action, wsConnPtr);
}