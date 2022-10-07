//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter check player specified by "playerId" and check player's permission
 * @param playerId: in attributes
 * @return void
 */

namespace techmino::filters {
    class CheckPermission :
            public drogon::HttpFilter<CheckPermission>,
            public helpers::I18nHelper<CheckPermission> {
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