//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/PlayerGroup.h>
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

PlayerGroup::PlayerGroup() : MessageHandlerBase(enum_integer(Action::PlayerGroup)) {}

optional<string> PlayerGroup::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->type != Player::Type::Gamer ||
        player->state != Player::State::Standby) {
        return i18n("notAvailable");
    }

    if (!request.check(JsonValue::UInt64)) {
        return i18n("invalidArguments");
    }
    return nullopt;
}

void PlayerGroup::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = player->playerId;
        data["group"] = request.ref().asUInt64();

        player->group = request.ref().asUInt64();
        player->getRoom()->publish(MessageJson(_action).setData(std::move(data)));
    }, _action, wsConnPtr);
}
