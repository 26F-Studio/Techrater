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
        unique_lock<shared_mutex> lock(_sharedMutex);
        if (_connectionMap.contains(playerId) &&
            _connectionMap[playerId]->connected()) {
            MessageJson(ErrorNumber::Error)
                    .setMessage(i18n("connectionReplaced"))
                    .to(_connectionMap[playerId]);
        }
        _connectionMap[playerId] = wsConnPtr;
    }
}

void ConnectionManager::unsubscribe(const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<PlayerBase>();
    if (wsConnPtr->connected() && player) {
        try {
            unique_lock<shared_mutex> lock(_sharedMutex);
            if (_connectionMap.at(player->playerId) == wsConnPtr) {
                _connectionMap.erase(player->playerId);
            } else {
                LOG_DEBUG << "Unsubscribe failed, not same connection: \n"
                          << "\tOriginal: (local: "
                          << _connectionMap.at(player->playerId)->localAddr().toIpPort()
                          << ", remote: "
                          << _connectionMap.at(player->playerId)->peerAddr().toIpPort()
                          << ")\n"
                          << "\tCurrent: (local: "
                          << wsConnPtr->localAddr().toIpPort()
                          << ", remote: "
                          << wsConnPtr->peerAddr().toIpPort()
                          << ")";
            }
        } catch (const out_of_range &e) {
            throw MessageException(i18n("playerNotFound"), e);
        }
    }
}

WebSocketConnectionPtr ConnectionManager::getConnPtr(int64_t userId) {
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        const auto &wsConnPtr = _connectionMap.at(userId);
        if (!wsConnPtr->connected() ||
            !wsConnPtr->getContext<PlayerBase>()) {
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
