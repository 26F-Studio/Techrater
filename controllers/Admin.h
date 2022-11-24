//
// Created by particleg on 2021/9/24.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/ConnectionManager.h>
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
            METHOD_ADD(
                    Admin::shutdownAfter,
                    "/shutdown/after",
                    drogon::Post,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::CheckPermission",
                    "techmino::filters::AdminShutdownAfter"
            );
        METHOD_LIST_END

        void shutdownAfter(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        plugins::ConnectionManager *_connectionManager;
    };
}