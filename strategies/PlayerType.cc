//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/PlayerType.h>
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

PlayerType::PlayerType() : MessageHandlerBase(enum_integer(Action::PlayerType)) {}

optional<string> PlayerType::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->state != Player::State::Standby) {
        return i18n("notAvailable");
    }

    if (!request.check(JsonValue::String)) {
        return i18n("invalidArguments");
    }

    const auto castedType = enum_cast<Player::Type>(request.ref().asString());
    if (!castedType.has_value()) {
        return i18n("invalidType");
    }
    if (castedType.value() == Player::Type::Gamer && player->getRoom()->full()) {
        return i18n("roomFull");
    }

    return nullopt;
}

void PlayerType::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = player->playerId;
        data["type"] = request.ref().asString();

        const auto type = enum_cast<Player::Type>(request.ref().asString()).value();
        player->type = type;
        auto room = player->getRoom();
        room->publish(MessageJson(_action).setData(std::move(data)));
        if (type == Player::Type::Spectator) {
            room->matchStart();
        } else {
            room->cancelStart();
        }
    }, _action, wsConnPtr);
}
