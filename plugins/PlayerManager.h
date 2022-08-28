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

        [[nodiscard]] int64_t getUserId(const std::string &accessToken);

        structures::RedisToken refresh(const std::string &refreshToken);

        void verifyEmail(const std::string &email);

        void verifyPhone(const std::string &phone);

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
                const std::string &accessToken,
                const std::string &newEmail,
                const std::string &code
        );

        void migratePhone(
                const std::string &accessToken,
                const std::string &newPhone,
                const std::string &code
        );

        void deactivateEmail(const std::string &accessToken, const std::string &code);

        void deactivatePhone(const std::string &accessToken, const std::string &code);

        [[nodiscard]] Json::Value getUserInfo(const std::string &accessToken, int64_t userId);

        void updateUserInfo(const std::string &accessToken, helpers::RequestJson request);

        [[nodiscard]] std::string getAvatar(const std::string &accessToken, int64_t userId);

        Json::Value getFollows(const std::string &accessToken, int64_t userId);

        bool follow(const std::string &accessToken, int64_t followId);

        Json::Value getStarred(const std::string &accessToken, int64_t userId);

        void dataStar(int64_t userId, int64_t dataId) const;

        void collectionStar(int64_t userId, int64_t collectionId) const;

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
