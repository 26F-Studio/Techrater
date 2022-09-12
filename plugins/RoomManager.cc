//
// Created by Parti on 2021/2/4.
//

#include <drogon/drogon.h>
#include <magic_enum.hpp>
#include <helpers/MessageJson.h>
#include <plugins/RoomManager.h>
#include <structures/Exceptions.h>
#include <structures/Player.h>
#include <types/Action.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;

RoomManager::RoomManager() = default;

void RoomManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "RoomManager loaded.";
}

void RoomManager::shutdown() { LOG_INFO << "RoomManager shutdown."; }

void RoomManager::playerConfig(int action, const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();

    Json::Value data;
    data["playerId"] = player->playerId;
    data["config"] = player->getConfig();
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.at(player->getRoomId()).publish(publishMessage);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerFinish(
        int action,
        const WebSocketConnectionPtr &wsConnPtr,
        Json::Value &&finishData
) {
    const auto &player = wsConnPtr->getContext<Player>();

    Json::Value data;
    data["playerId"] = player->playerId;
    data["finishData"] = move(finishData);
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto &room = _roomMap.at(player->getRoomId());
        room.publish(publishMessage);
        room.tryEnd();
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerGroup(int action, const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();

    Json::Value data;
    data["playerId"] = player->playerId;
    data["group"] = player->group.load();
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.at(player->getRoomId()).publish(publishMessage);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerReady(int action, const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();
    auto ready = player->state == Player::State::Ready;

    Json::Value data;
    data["playerId"] = player->playerId;
    data["ready"] = ready;
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto &room = _roomMap.at(player->getRoomId());
        room.publish(publishMessage);
        if (ready) {
            room.tryStart();
        } else {
            room.tryCancelStart();
        }
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerRole(int action, const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();

    Json::Value data;
    data["playerId"] = player->playerId;
    data["role"] = string(enum_name(player->role.load()));
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.at(player->getRoomId()).publish(publishMessage);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerState(int action, const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();

    Json::Value data;
    data["playerId"] = player->playerId;
    data["customState"] = player->getCustomState();
    MessageJson publishMessage(action);
    publishMessage.setData(move(data));
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.at(player->getRoomId()).publish(publishMessage);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

void RoomManager::playerType(
        int action,
        const WebSocketConnectionPtr &wsConnPtr,
        Player::Type type
) {
    const auto &player = wsConnPtr->getContext<Player>();
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto &room = _roomMap.at(player->getRoomId());
        if (type == Player::Type::Gamer && room.full()) {
            MessageJson failedMessage(action);
            failedMessage.setMessageType(MessageType::Failed);
            failedMessage.setReason(i18n("roomFull"));
            failedMessage.sendTo(wsConnPtr);
        } else {
            player->type = type;

            Json::Value data;
            data["playerId"] = player->playerId;
            data["type"] = string(enum_name(player->type.load()));
            MessageJson publishMessage(action);
            publishMessage.setData(move(data));
            room.publish(publishMessage);
        }
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

Json::Value RoomManager::listRoom(uint64_t pageIndex, uint64_t pageSize) const {
    Json::Value data(Json::arrayValue);
    {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto iterator = _roomMap.begin();
        for (uint64_t i = 0; i < pageIndex * pageSize; ++i) {
            if (iterator == _roomMap.end()) {
                break;
            }
            ++iterator;
        }

        for (uint64_t i = 0; i < pageSize; ++i) {
            if (iterator == _roomMap.end()) {
                break;
            }
            data.append(iterator->second->parse());
            ++iterator;
        }
    }
    return data;
}

void RoomManager::roomPassword(
        int action,
        const WebSocketConnectionPtr &wsConnPtr,
        string &&password
) {
    const auto &player = wsConnPtr->getContext<Player>();
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        auto &room = _roomMap.at(player->getRoomId());

        room.updatePassword(password);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
    MessageJson successMessage(action);
    successMessage.setMessageType(MessageType::Server);
    successMessage.sendTo(wsConnPtr);
}

RoomPtr RoomManager::getRoom(const std::string &roomId) const {
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        return _roomMap.at(roomId);
    } catch (const out_of_range &) {
        throw MessageException("roomNotFound");
    }
}

bool RoomManager::removeRoom(const string &roomId) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    return _roomMap.erase(roomId);
}

bool RoomManager::setRoom(const RoomPtr &room) {
    if (room) {
        unique_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.emplace(room->roomId, std::move(room));
        return true;
    }
    return false;
}
