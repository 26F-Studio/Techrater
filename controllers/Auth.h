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
            METHOD_ADD(Auth::deactivate, "/deactivate/{1}", drogon::Post, "techmino::filters::AuthDeactivate");
            METHOD_ADD(Auth::oauth, "/oauth/{1}", drogon::Post, "techmino::filters::AuthOauth");
        METHOD_LIST_END

        void check(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

        void deactivate(
            const drogon::HttpRequestPtr& req,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string&& token
        ) const;

        void oauth(
            const drogon::HttpRequestPtr& req,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string&& token
        ) const;

    private:
        plugins::PlayerManager* _playerManager;
    };
}
