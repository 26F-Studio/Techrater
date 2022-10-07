//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks param "lastCount", "language" or "pageIndex", "pageSize", "language" and set attributes
 * @param lastCount: in params, optional
 * @param pageIndex: in params, optional
 * @param pageSize: in params, optional
 * @param language: in params
 * @return lastCount: in request attributes, optional
 * @return pageIndex: in request attributes, optional
 * @return pageSize: in request attributes, optional
 * @return language: in request attributes
 */

namespace techmino::filters {
    class CheckAccessToken :
            public drogon::HttpFilter<CheckAccessToken>,
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