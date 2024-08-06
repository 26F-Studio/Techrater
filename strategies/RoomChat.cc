//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomChat.h>
#include <types/Action.h>
#include <types/JsonValue.h>
#include <types/Permission.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

RoomChat::RoomChat() : MessageHandlerBase(enum_integer(Action::RoomChat)) {}

optional<string> RoomChat::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();

    if (!player->getRoom()) {
        return i18n("roomNotFound");
    }

    /// @note Reject if the message is not an string.
    if (!request.check("message", JsonValue::String)) {
        return i18n("invalidArguments");
    }

    return nullopt;
}

void RoomChat::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        RoomPtr room;
        if (request.check("roomId", JsonValue::String)) {
            room = app().getPlugin<RoomManager>()->getRoom(request["roomId"].asString());
        } else {
            room = player->getRoom();
        }
        Json::Value data;
        data["playerId"] = player->playerId;
        data["message"] = request["message"].asString();

        room->publish(MessageJson(_action).setData(data));
        room->appendChat(std::move(data));
    }, _action, wsConnPtr);
}