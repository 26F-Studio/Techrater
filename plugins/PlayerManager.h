//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <helpers/RequestJson.h>
#include <models/Player.h>
#include <structures/PlayerRedis.h>

namespace techmino::plugins {
    class PlayerManager :
            public drogon::Plugin<PlayerManager>,
            public helpers::I18nHelper<PlayerManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        [[nodiscard]] int64_t getPlayerId(const std::string &accessToken);

        structures::RedisToken refresh(const std::string &refreshToken);

        void verifyEmail(const std::string &email);

        void verifyPhone(const std::string &phone);

        std::string seedEmail(const std::string &email);

        std::string seedPhone(const std::string &phone);

        [[nodiscard]] std::tuple<structures::RedisToken, bool> loginEmailCode(
                const std::string &email,
                const std::string &code
        );

        [[nodiscard]] std::tuple<structures::RedisToken, bool> loginPhoneCode(
                const std::string &phone,
                const std::string &code
        );

        [[nodiscard]] structures::RedisToken loginEmailPassword(
                const std::string &email,
                const std::string &password
        );

        [[nodiscard]] structures::RedisToken loginPhonePassword(
                const std::string &phone,
                const std::string &password
        );

        void resetEmail(
                const std::string &email,
                const std::string &code,
                const std::string &newPassword
        );

        void resetPhone(
                const std::string &phone,
                const std::string &code,
                const std::string &newPassword
        );

        void migrateEmail(
                int64_t userId,
                const std::string &newEmail,
                const std::string &code
        );

        void migratePhone(
                int64_t userId,
                const std::string &newPhone,
                const std::string &code
        );

        void deactivateEmail(
                int64_t userId,
                const std::string &code
        );

        void deactivatePhone(
                int64_t userId,
                const std::string &code
        );

        [[nodiscard]] Json::Value getUserInfo(
                const std::string &accessToken,
                int64_t userId
        );

        void updateUserInfo(
                int64_t userId,
                helpers::RequestJson request
        );

        [[nodiscard]] std::string getAvatar(
                const std::string &accessToken,
                int64_t userId
        );

        [[nodiscard]] bool ipLimit(const std::string &ip) const;

        [[nodiscard]] bool emailLimit(const std::string &email) const;

        [[nodiscard]] bool phoneLimit(const std::string &phone) const;

    private:
        std::chrono::seconds _ipInterval{}, _emailInterval{}, _phoneInterval{};
        uint64_t _ipMaxCount{}, _emailMaxCount{}, _phoneMaxCount{};

        std::unique_ptr<techmino::structures::PlayerRedis> _userRedis;
        std::unique_ptr<drogon::orm::Mapper<drogon_model::techrater::Player>> _playerMapper;

        void _checkEmailCode(const std::string &email, const std::string &code);

        void _checkPhoneCode(const std::string &phone, const std::string &code);
    };
}
