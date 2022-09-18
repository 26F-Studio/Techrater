//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter trims "User::updateData" request body
 * @param statistics: String
 * @param ranks: String
 * @param settings: String
 * @param keymaps: String
 * @param touch_1: String
 * @param touch_2: String
 * @param touch_3: String
 * @param extra_1: String
 * @param extra_2: String
 * @param extra_3: String
 * @return requestJson: in request attributes
 */
namespace techmino::filters {
    class PlayerUpdateData :
            public drogon::HttpFilter<PlayerUpdateData>,
            public structures::RequestJsonHandler<PlayerUpdateData> {
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