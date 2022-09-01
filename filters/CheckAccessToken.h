//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter sends header "x-access-token" to User node and set attribute "playerId"
 * @param x-access-token: in header
 * @return playerId: int64_t
 */

namespace techmino::filters {
    class CheckAccessToken :
            public drogon::HttpFilter<CheckAccessToken, false>,
            public helpers::I18nHelper<CheckAccessToken> {
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