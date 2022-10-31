//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <strategies/PlayerConfig.h>
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

PlayerConfig::PlayerConfig() : MessageHandlerBase(enum_integer(Action::PlayerConfig)) {}

optional<string> PlayerConfig::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    if (!request.check(JsonValue::String)) {
        return i18n("invalidArguments");
    }
    return nullopt;
}

void PlayerConfig::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        player->setConfig(request.ref().asString());
        if (player->getRoom()) {
            Json::Value data;
            data["playerId"] = player->playerId;
            data["config"] = request.ref().asString();
            player->getRoom()->publish(MessageJson(_action).setData(std::move(data)));
        } else {
            MessageJson(_action).setData(request.ref().asString()).to(wsConnPtr);
        }
    }, _action, wsConnPtr);
}
