//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/ConnectionManager.h>
#include <plugins/RoomManager.h>
#include <strategies/PlayerRole.h>
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

PlayerRole::PlayerRole() : MessageHandlerBase(enum_integer(Action::PlayerRole)) {}

optional<string> PlayerRole::filter(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    if (!player->getRoom() ||
        player->role < Player::Role::Admin ||
        player->state != Player::State::Standby) {
        return i18n("notAvailable");
    }

    /// @note Reject if no playerId or role is specified.
    if (!request.check("playerId", JsonValue::Int64) ||
        request["playerId"].asInt64() == player->playerId ||
        !request.check("role", JsonValue::String)) {
        return i18n("invalidArguments");
    }

    const auto &targetPlayer = app().getPlugin<ConnectionManager>()->getConnPtr(
            request["playerId"].asInt64()
    )->getContext<Player>();
    const auto castedRole = enum_cast<Player::Role>(request["role"].asString());
    /// @note Reject if role is invalid.
    if (!castedRole.has_value()) {
        return i18n("invalidRole");
    }
    /// @note Reject if given role is higher than the executor,
    ///       or the executor's role is lower than the target's.
    if (player->role < castedRole.value() ||
        player->role <= targetPlayer->role) {
        return i18n("noPermission");
    }
    if (player->getRoom()->roomId != targetPlayer->getRoom()->roomId) {
        return i18n("invalidTarget");
    }
    return nullopt;
}

void PlayerRole::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    const auto &player = wsConnPtr->getContext<Player>();
    const auto &targetPlayer = app().getPlugin<ConnectionManager>()->getConnPtr(
            request["playerId"].asInt64()
    )->getContext<Player>();
    const auto targetRole = enum_cast<Player::Role>(request["role"].asString()).value();
    handleExceptions([&]() {
        const auto room = player->getRoom();
        if (player->role == targetRole) {
            /// @note Switching roles
            const auto executorRole = targetPlayer->role.load();
            player->role = executorRole;
            targetPlayer->role = targetRole;

            Json::Value data;
            data["playerId"] = targetPlayer->playerId;
            data["role"] = string(enum_name(targetRole));
            room->publish(MessageJson(_action).setData(data));
            data["playerId"] = player->playerId;
            data["role"] = string(enum_name(executorRole));
            room->publish(MessageJson(_action).setData(data));
        } else {
            targetPlayer->role = targetRole;

            Json::Value data;
            data["playerId"] = targetPlayer->playerId;
            data["role"] = string(enum_name(targetRole));
            room->publish(MessageJson(_action).setData(data));
        }
    }, _action, wsConnPtr);
}
