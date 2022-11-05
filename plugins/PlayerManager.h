//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <models/Data.h>
#include <models/Player.h>
#include <structures/PlayerRedis.h>

namespace techmino::plugins {
    class PlayerManager :
            public drogon::Plugin<PlayerManager>,
            public helpers::I18nHelper<PlayerManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        PlayerManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        [[nodiscard]] int64_t getPlayerId(const std::string &accessToken);

        structures::RedisToken refresh(const std::string &refreshToken);

        void verifyEmail(
                const std::string &email,
                const std::function<void(const drogon::HttpResponsePtr &)> &callback
        );

        std::string seedEmail(const std::string &email);

        [[nodiscard]] std::tuple<structures::RedisToken, bool> loginEmailCode(
                const std::string &email,
                const std::string &code
        );

        [[nodiscard]] structures::RedisToken loginEmailPassword(
                const std::string &email,
                const std::string &password
        );

        void resetEmail(
                const std::string &email,
                const std::string &code,
                const std::string &newPassword
        );

        void migrateEmail(
                int64_t playerId,
                const std::string &newEmail,
                const std::string &code
        );

        void deactivateEmail(
                int64_t playerId,
                const std::string &code
        );

        [[nodiscard]] std::string getAvatar(
                const std::string &accessToken,
                int64_t playerId
        );

        [[nodiscard]] Json::Value getPlayerInfo(
                const std::string &accessToken,
                int64_t playerId
        );

        void updatePlayerInfo(
                int64_t playerId,
                helpers::RequestJson request
        );

        [[nodiscard]] Json::Value getPlayerData(
                const std::string &accessToken,
                int64_t playerId
        );

        void updatePlayerData(
                int64_t playerId,
                helpers::RequestJson request
        );

        [[nodiscard]] bool ipLimit(const std::string &ip) const;

        [[nodiscard]] bool loginLimit(const std::string &type, const std::string &key) const;

        [[nodiscard]] bool verifyLimit(const std::string &type, const std::string &key) const;

    private:
        std::chrono::seconds _ipInterval{}, _verifyInterval{}, _loginInterval{};
        uint64_t _ipMaxCount{}, _verifyMaxCount{}, _loginMaxCount{};

        std::unique_ptr<techmino::structures::PlayerRedis> _playerRedis;
        drogon::orm::Mapper<drogon_model::techrater::Data> _dataMapper;
        drogon::orm::Mapper<drogon_model::techrater::Player> _playerMapper;

        void _checkEmailCode(const std::string &email, const std::string &code);
    };
}
