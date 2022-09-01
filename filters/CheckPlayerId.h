//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks param "playerId" and set attribute "playerId"
 * @param id: in query string
 * @return playerId: Int64
 */

namespace techmino::filters {
    class CheckPlayerId :
            public drogon::HttpFilter<CheckPlayerId>,
            public helpers::I18nHelper<CheckPlayerId> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

        void doFilter(
                const drogon::HttpRequestPtr &req,
                drogon::FilterCallback &&failedCb,
                drogon::FilterChainCallback &&nextCb
        ) override;
    };
}