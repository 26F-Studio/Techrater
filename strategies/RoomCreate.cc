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

bool RoomCreate::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player || !player->getRoom()) {
        MessageJson message(_action);
        message.setMessageType(MessageType::Failed);
        message.setReason(i18n("notAvailable"));
        message.sendTo(wsConnPtr);
        return false;
    }

    if (!request.check("capacity", JsonValue::Uint64) ||
        !request.check("info", JsonValue::Object) ||
        !request.check("data", JsonValue::Object) ||
        request["capacity"].asUInt64() <= 0) {
        MessageJson message(_action);
        message.setMessageType(MessageType::Failed);
        message.setReason(i18n("invalidArguments"));
        message.sendTo(wsConnPtr);
        return false;
    }
    return true;
}

void RoomCreate::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) {
    handleExceptions([&]() {
        string password;
        if (request.check("password", JsonValue::String)) {
            password = std::move(request["password"].asString());
        }

        app().getPlugin<RoomManager>()->roomCreate(
                _action,
                wsConnPtr,
                request["capacity"].asUInt64(),
                std::move(password),
                request["info"],
                request["data"]
        );
    }, _action, wsConnPtr);
}