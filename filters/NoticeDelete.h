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
    class NoticeDelete :
            public drogon::HttpFilter<NoticeDelete>,
            public helpers::I18nHelper<NoticeDelete> {
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