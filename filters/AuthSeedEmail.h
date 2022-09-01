//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter checks "Auth::seedEmail" request body.
 * @param email: String
 * @return requestJson: in request attributes
 */

namespace techmino::filters {
    class AuthSeedEmail :
            public drogon::HttpFilter<AuthSeedEmail>,
            public structures::RequestJsonHandler<AuthSeedEmail> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        void doFilter(
                const drogon::HttpRequestPtr &req,
                drogon::FilterCallback &&failedCb,
                drogon::FilterChainCallback &&nextCb
        ) override;
    };
}