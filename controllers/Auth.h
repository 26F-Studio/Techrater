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
            ADD_METHOD_TO(Auth::oauth, "/auth/oauth", drogon::Post, "techmino::filters::AuthOauth");
            METHOD_ADD(Auth::check, "/check", drogon::Get, "techmino::filters::CheckAccessToken");
        METHOD_LIST_END

        void oauth(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void check(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        plugins::PlayerManager *_playerManager;
    };
}