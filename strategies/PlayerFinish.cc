//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <strategies/PlayerFinish.h>
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

PlayerFinish::PlayerFinish() : MessageHandlerBase(enum_integer(Action::PlayerFinish)) {}

optional<string> PlayerFinish::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->type != Player::Type::Gamer ||
        player->state != Player::State::Playing) {
        return i18n("notAvailable");
    }

    if (!request.check(JsonValue::Object)) {
        return i18n("invalidArguments");
    }
    return nullopt;
}

void PlayerFinish::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = player->playerId;
        data["data"] = std::move(request.ref());

        player->state = Player::State::Finished;
        auto room = player->getRoom();
        room->publish(MessageJson(_action).setData(std::move(data)));
        room->matchTryEnd();
    }, _action, wsConnPtr);
}
