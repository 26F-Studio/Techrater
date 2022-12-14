//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/PlayerReady.h>
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

PlayerReady::PlayerReady() : MessageHandlerBase(enum_integer(Action::PlayerReady)) {}

optional<string> PlayerReady::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->type != Player::Type::Gamer ||
        player->state > Player::State::Ready) {
        return i18n("notAvailable");
    }

    if (!request.check(JsonValue::Bool)) {
        return i18n("invalidArguments");
    }
    return nullopt;
}

void PlayerReady::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    const auto isReady = request.ref().asBool();
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = player->playerId;
        data["isReady"] = isReady;

        player->state = isReady ? Player::State::Ready : Player::State::Standby;
        auto room = player->getRoom();
        room->publish(MessageJson(_action).setData(std::move(data)));
        if (isReady) {
            room->matchTryStart();
        } else {
            room->cancelStart();
        }
    }, _action, wsConnPtr);
}
