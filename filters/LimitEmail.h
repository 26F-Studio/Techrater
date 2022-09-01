//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter limits request rate from specific email address
 * @param email: String
 */
namespace techmino::filters {
    class LimitEmail :
            public drogon::HttpFilter<LimitEmail>,
            public helpers::I18nHelper<LimitEmail> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

        void doFilter(
                const drogon::HttpRequestPtr &req,
                drogon::FilterCallback &&failedCb,
                drogon::FilterChainCallback &&nextCb
        ) override;
    };
}