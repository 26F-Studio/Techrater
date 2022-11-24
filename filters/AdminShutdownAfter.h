//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter checks "Admin::shutdown" request body.
 * @param delay: Double
 * @return requestJson: in request attributes
 */

namespace techmino::filters {
    class AdminShutdownAfter :
            public drogon::HttpFilter<AdminShutdownAfter>,
            public structures::RequestJsonHandler<AdminShutdownAfter> {
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