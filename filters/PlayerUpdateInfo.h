//
// Created by particleg on 2021/9/27.
//

#pragma once

#include <drogon/HttpFilter.h>
#include <structures/ExceptionHandlers.h>

/**
 * @brief This filter trims "User::updateInfo" request body
 * @param username: String
 * @param motto: String
 * @param region: Int64
 * @param avatar: String
 * @param avatar_frame: Int64
 * @param clan: Int64
 * @param password: String
 * @return requestJson: in request attributes
 */
namespace techmino::filters {
    class PlayerUpdateInfo :
            public drogon::HttpFilter<PlayerUpdateInfo>,
            public structures::RequestJsonHandler<PlayerUpdateInfo> {
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