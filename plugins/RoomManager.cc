//
// Created by Parti on 2021/2/4.
//

#include <drogon/drogon.h>
#include <plugins/RoomManager.h>
#include <structures/Exceptions.h>
#include <structures/Player.h>

using namespace drogon;
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

RoomPtr RoomManager::getRoom(const std::string &roomId) const {
    try {
        shared_lock<shared_mutex> lock(_sharedMutex);
        return _roomMap.at(roomId);
    } catch (const out_of_range &e) {
        throw MessageException(i18n("roomNotFound"), e);
    }
}

bool RoomManager::removeRoom(const string &roomId) {
    unique_lock<shared_mutex> lock(_sharedMutex);
    return _roomMap.erase(roomId);
}

bool RoomManager::setRoom(RoomPtr &&room) {
    if (room) {
        unique_lock<shared_mutex> lock(_sharedMutex);
        _roomMap.emplace(room->roomId, std::move(room));
        return true;
    }
    return false;
}
