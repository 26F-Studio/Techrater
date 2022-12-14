//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomLeave.h>
#include <structures/Player.h>
#include <types/Action.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

RoomLeave::RoomLeave() : MessageHandlerBase(enum_integer(Action::RoomLeave)) {}

optional<string> RoomLeave::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    if (!wsConnPtr->getContext<Player>()->getRoom()) {
        return i18n("notAvailable");
    }
    return nullopt;
}

void RoomLeave::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &roomManager = app().getPlugin<RoomManager>();
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = roomManager->getRoom(request["roomId"].asString());
        } else {
            room = wsConnPtr->getContext<Player>()->getRoom();
        }

        Json::Value data;
        data["playerId"] = player->playerId;
        const auto message = MessageJson(_action).setData(std::move(data));
        room->unsubscribe(player->playerId, message);

        player->reset();
        message.to(wsConnPtr);
    }, _action, wsConnPtr);
}
