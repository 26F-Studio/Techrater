//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/RedisHelper.h>
#include <helpers/RequestJson.h>
#include <models/Data.h>
#include <structures/ExceptionHandlers.h>

namespace techmino::plugins {
    class PlayerManager :
            public drogon::Plugin<PlayerManager>,
            public structures::HttpRequestHandler<PlayerManager>,
            public helpers::RedisHelper {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        PlayerManager();

        void initAndStart(const Json::Value& config) override;

        void shutdown() override;

        void deactivate(int64_t playerId, const std::string& token);

        std::string oauth(int64_t playerId, const std::string& token);

        int64_t getPlayerIdByAccessToken(const std::string& accessToken);

        bool tryRefresh(std::string& accessToken);

        Json::Value getPlayerData(
            const std::string& accessToken,
            int64_t playerId
        );

        void updatePlayerData(
            int64_t playerId,
            helpers::RequestJson request
        );

        bool ipLimit(const std::string& ip);

        bool loginLimit(const std::string& type, const std::string& key);

        bool verifyLimit(const std::string& type, const std::string& key);

    private:
        std::string _authToken;
        std::chrono::seconds _ipInterval{}, _verifyInterval{}, _loginInterval{},
                _accessExpiration{}, _refreshExpiration{};
        uint64_t _ipMaxCount{}, _verifyMaxCount{}, _loginMaxCount{};

        drogon::orm::Mapper<drogon_model::techrater::Data> _dataMapper;

        std::string _generateAccessToken(const std::string& playerId);
    };
}
