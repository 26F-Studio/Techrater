//
// Created by particleg on 2023/7/25.
//

#pragma once

#include <cryptopp/cryptlib.h>
#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <structures/Player.h>
#include <types/Action.h>

namespace techmino::plugins {
    class LeagueManager :
            public drogon::Plugin<LeagueManager>,
            public helpers::I18nHelper<LeagueManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;
    public:
        LeagueManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override; 

        bool joinQueue(const std::shared_ptr<structures::Player> &player);

        bool leaveQueue(const std::shared_ptr<structures::Player> &player);
    };
}