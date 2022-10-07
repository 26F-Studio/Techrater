//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter trims "Notice::CreateNotice" request body
 * @param en_us: String
 * @param zh_cn: String
 * @param zh_tw: String
 * @param fr_fr: String
 * @param es_es: String
 * @param pt_pt: String
 * @param id_id: String
 * @param ja_jp: String
 * @return contents: in request attributes
 */
namespace techmino::filters {
    class NoticeUpdate :
            public drogon::HttpFilter<NoticeUpdate>,
            public structures::RequestJsonHandler<NoticeUpdate> {
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