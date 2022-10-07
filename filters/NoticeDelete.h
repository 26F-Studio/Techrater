//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <helpers/I18nHelper.h>

/**
 * @brief This filter checks param "noticeId" and set attribute "noticeId"
 * @param noticeId: Int64
 * @return noticeId: in request attributes
 */
namespace techmino::filters {
    class PlayerUpdateInfo :
            public drogon::HttpFilter<PlayerUpdateInfo>,
            public helpers::I18nHelper<PlayerUpdateInfo> {
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