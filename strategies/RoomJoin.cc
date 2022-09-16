//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomJoin.h>
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

RoomJoin::RoomJoin() : MessageHandlerBase(enum_integer(Action::RoomJoin)) {}

optional<string> RoomJoin::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    if (wsConnPtr->getContext<Player>()->getRoom()) {
        return i18n("notAvailable");
    }

    if (!request.check("roomId", JsonValue::String)) {
        return i18n("invalidArguments");
    }

    request.trim("password", JsonValue::String);

    return nullopt;
}

void RoomJoin::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        auto room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        if (room->checkPassword(request["password"].asString())) {
            bool spectate = false;
            if (room->state == Room::State::Playing) {
                spectate = true;
            } else if (!room->full() && room->cancelStart()) {
                player->type = Player::Type::Gamer;
            }
            player->setRoom(room);
            MessageJson(_action)
                                .setMessageType(MessageType::Server)
                                .setData(room->parse(true))
                                .sendTo(wsConnPtr);

            room->subscribe(player->playerId);
            room->publish(
                    MessageJson(_action).setData(player->info()),
                    player->playerId
            );

            if (spectate) {
                // TODO: Implement spectate logics
            }
        } else {
            throw MessageException("wrongPassword");
        }
    }, _action, wsConnPtr);
}