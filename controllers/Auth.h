//
// Created by particleg on 2021/9/24.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>

namespace techmino::api::v1 {
    class Auth :
            public drogon::HttpController<Auth>,
            public structures::ResponseJsonHandler<Auth> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        Auth();

        METHOD_LIST_BEGIN
            METHOD_ADD(Auth::check, "/check", drogon::Get, "techmino::filters::CheckAccessToken");
            METHOD_ADD(Auth::refresh, "/refresh", drogon::Get, "techmino::filters::CheckRefreshToken");
            METHOD_ADD(
                    Auth::verifyEmail,
                    "/verify/email",
                    drogon::Post,
                    "techmino::filters::AuthVerifyEmail",
                    "techmino::filters::LimitVerifyEmail",
                    "techmino::filters::LimitIp"
            );
            METHOD_ADD(Auth::seedEmail, "/seed/email", drogon::Post, "techmino::filters::AuthSeedEmail");
            METHOD_ADD(
                    Auth::loginEmail,
                    "/login/email",
                    drogon::Post,
                    "techmino::filters::AuthLoginEmail",
                    "techmino::filters::LimitLoginEmail",
                    "techmino::filters::LimitIp"
            );
            METHOD_ADD(
                    Auth::resetEmail,
                    "/reset/email",
                    drogon::Put,
                    "techmino::filters::AuthResetEmail",
                    "techmino::filters::LimitLoginEmail",
                    "techmino::filters::LimitIp"
            );
            METHOD_ADD(
                    Auth::migrateEmail,
                    "/migrate/email",
                    drogon::Put,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::AuthMigrateEmail");
            METHOD_ADD(
                    Auth::deactivateEmail,
                    "/deactivate/email",
                    drogon::Post,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::AuthDeactivateEmail");
        METHOD_LIST_END

        void check(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void refresh(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void verifyEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void seedEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void loginEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void resetEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void migrateEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void deactivateEmail(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        plugins::PlayerManager *_playerManager;
    };
}