//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter checks "Auth::oauth" request body.
 * @param playerId: Int64
 * @return requestJson: in request attributes
 */

namespace techmino::filters {
    class AuthDeactivate :
            public drogon::HttpFilter<AuthDeactivate>,
            public structures::RequestJsonHandler<AuthDeactivate> {
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