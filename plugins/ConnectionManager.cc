//
// Created by g29624 on 2022/8/29.
//

#include <plugins/ConnectionManager.h>
#include <structures/Exceptions.h>
#include <structures/PlayerBase.h>

using namespace drogon;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

void ConnectionManager::initAndStart(const Json::Value &config) {
    LOG_INFO << "ConnectionManager loaded.";
}

void ConnectionManager::shutdown() { LOG_INFO << "ConnectionManager shutdown."; }

void ConnectionManager::subscribe(const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<PlayerBase>();
    if (wsConnPtr->connected() && player) {
        const auto playerId = player->playerId;
        LOG_DEBUG << "Player " << playerId << " subscribing...";
        unsubscribe(wsConnPtr, true);
        {
            unique_lock<shared_mutex> lock(_sharedMutex);
            _connectionMap[playerId] = wsConnPtr;

            // TODO: Remove debugging logs
            string playerList;
            for (const auto &[_playerId, _]: _connectionMap) {
                playerList += to_string(_playerId) + ", ";
            }
            LOG_DEBUG << "_connectionMap(" << _connectionMap.size() << "): " + playerList;
        }
    } else {
        LOG_WARN << "Invalid connection: " << wsConnPtr->peerAddr().toIpPort();
    }
}

void ConnectionManager::unsubscribe(const WebSocketConnectionPtr &wsConnPtr, bool force) {
    const auto playerId = wsConnPtr->getContext<PlayerBase>()->playerId;
    try {
        unique_lock<shared_mutex> lock(_sharedMutex);
        const auto oldWsConnPtr = _connectionMap.at(playerId);
        if (force || oldWsConnPtr == wsConnPtr) {
            if (oldWsConnPtr->connected()) {
                MessageJson(ErrorNumber::Error)
                        .setMessage(i18n("connectionClosed"))
                        .to(oldWsConnPtr);
            } else {
                oldWsConnPtr->forceClose();
            }
            LOG_DEBUG << "Erasing connection of player " << playerId << "...";
            auto result = _connectionMap.erase(playerId);
            LOG_DEBUG << "Erased " << result << " connections.";

            // TODO: Remove debugging logs
            string playerList;
            for (const auto &[_playerId, _]: _connectionMap) {
                playerList += to_string(_playerId) + ", ";
            }
            LOG_DEBUG << "_connectionMap(" << _connectionMap.size() << "): " + playerList;
        }
    } catch (const exception &e) {
        LOG_TRACE << "Exception caught: " << e.what();
    }
}

void ConnectionManager::broadcast(const MessageJson &message) {
    shared_lock<shared_mutex> lock(_sharedMutex);
    for (const auto &[_, wsConnPtr]: _connectionMap) {
        message.to(wsConnPtr);
    }
}

WebSocketConnectionPtr ConnectionManager::getConnPtr(int64_t playerId) {
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        const auto &wsConnPtr = _connectionMap.at(playerId);
        if (!wsConnPtr->connected() || !wsConnPtr->getContext<PlayerBase>()) {
            LOG_DEBUG << "Invalid player: " << playerId;
            lock.unlock();
            unsubscribe(wsConnPtr);
            throw MessageException(i18n("playerInvalid"));
        }
        return wsConnPtr;
    } catch (const out_of_range &e) {
        throw MessageException(i18n("playerNotFound"), e);
    }
}

uint64_t ConnectionManager::count() const {
    shared_lock<shared_mutex> lock(_sharedMutex);
    return _connectionMap.size();
}
