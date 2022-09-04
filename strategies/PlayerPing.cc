//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <strategies/PlayerPing.h>
#include <structures/Player.h>
#include <types/Action.h>
#include <types/JsonValue.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

PlayerPing::PlayerPing() : MessageHandlerBase(enum_integer(Action::PlayerPing)) {}

bool PlayerPing::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player) {
        MessageJson message(_action);
        message.setMessageType(MessageType::Failed);
        message.setReason(i18n("notAvailable"));
        message.sendTo(wsConnPtr);
        return false;
    }

    if (!request.check(JsonValue::Array)) {
        MessageJson message(_action);
        message.setMessageType(MessageType::Failed);
        message.setReason(i18n("invalidArguments"));
        message.sendTo(wsConnPtr);
        return false;
    }
    return true;
}

void PlayerPing::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) {
    handleExceptions([&]() {
        wsConnPtr->getContext<Player>()->setPingList(move(request.ref()));
    }, _action, wsConnPtr);
}
