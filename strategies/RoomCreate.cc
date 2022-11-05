//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomCreate.h>
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

RoomCreate::RoomCreate() : MessageHandlerBase(enum_integer(Action::RoomCreate)) {}

optional<string> RoomCreate::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    if (!wsConnPtr->getContext<Player>()->hasConfig()) {
        return i18n("notAvailable");
    }

    if (!request.check("capacity", JsonValue::UInt64) ||
        !request.check("info", JsonValue::Object) ||
        !request.check("data", JsonValue::Object) ||
        request["capacity"].asUInt64() == 0) {
        return i18n("invalidArguments");
    }

    const auto &player = wsConnPtr->getContext<Player>();
    const auto &room = player->getRoom();
    if (room) {
        Json::Value data;
        data["playerId"] = player->playerId;
        const auto message = MessageJson(enum_integer(Action::RoomLeave)).setData(std::move(data));
        room->unsubscribe(player->playerId, message);

        player->reset();
        message.to(wsConnPtr);
    }

    request.trim("password", JsonValue::String);

    return nullopt;
}

void RoomCreate::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    handleExceptions([&]() {
        auto room = make_shared<Room>(
                request["capacity"].asUInt64(),
                std::move(request["password"].asString()),
                request["info"],
                request["data"]
        );
        room->subscribe(wsConnPtr);

        player->setRoom(room);
        player->role = Player::Role::Admin;
        player->type = Player::Type::Gamer;
        MessageJson(_action).setData(room->parse(true)).to(wsConnPtr);

        app().getPlugin<RoomManager>()->setRoom(std::move(room));
    }, _action, wsConnPtr);
}