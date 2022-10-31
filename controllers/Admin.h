//
// Created by particleg on 2021/9/24.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>

namespace techmino::api::v1 {
    class Admin :
            public drogon::HttpController<Admin>,
            public structures::ResponseJsonHandler<Admin> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        Admin();

        METHOD_LIST_BEGIN
            METHOD_ADD(Admin::shutdown, "/shutdown", drogon::Post, "techmino::filters::CheckAccessToken");
        METHOD_LIST_END

        void shutdown(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        plugins::PlayerManager *_playerManager;
    };
}