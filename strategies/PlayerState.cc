//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/PlayerState.h>
#include <structures/Player.h>
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

PlayerState::PlayerState() : MessageHandlerBase(enum_integer(Action::PlayerState)) {}

optional<string> PlayerState::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->type != Player::Type::Gamer ||
        player->state != Player::State::Playing) {
        return i18n("notAvailable");
    }

    if (!request.check(JsonValue::String)) {
        return i18n("invalidArguments");
    }
    return nullopt;
}

void PlayerState::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = player->playerId;
        data["customState"] = request.ref().asString();

        player->setCustomState(std::move(request.ref().asString()));
        player->getRoom()->publish(MessageJson(_action).setData(std::move(data)));
    }, _action, wsConnPtr);
}
