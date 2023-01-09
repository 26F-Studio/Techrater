//
// Created by particleg on 2021/9/24.
//

#pragma once

#include <drogon/HttpController.h>
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
        METHOD_LIST_END

        void check(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    };
}