//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks header "x-access-token" and set attribute "accessToken"
 * @param x-access-token: in header
 * @return accessToken: String
 */

namespace techmino::filters {
    class GetAccessToken :
            public drogon::HttpFilter<GetAccessToken>,
            public helpers::I18nHelper<GetAccessToken> {
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