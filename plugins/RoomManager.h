//
// Created by Parti on 2021/2/4.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <structures/Room.h>
#include <structures/Player.h>
#include <types/Action.h>

namespace {
    using RoomPtr = std::shared_ptr<techmino::structures::Room>;
}

namespace techmino::plugins {
    class RoomManager :
            public drogon::Plugin<RoomManager>,
            public helpers::I18nHelper<RoomManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        RoomManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        Json::Value listRoom(uint64_t pageIndex, uint64_t pageSize) const;

        RoomPtr getRoom(const std::string &roomId) const;

        bool removeRoom(const std::string &roomId);

        bool setRoom(const RoomPtr &room);

    private:
        mutable std::shared_mutex _sharedMutex;
        std::unordered_map<std::string, RoomPtr> _roomMap;
    };
}

