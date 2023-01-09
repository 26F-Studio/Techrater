//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckAccessToken.h>
#include <helpers/ResponseJson.h>
#include <plugins/PlayerManager.h>
#include <structures/Exceptions.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

void CheckAccessToken::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto accessToken = req->getHeader("x-access-token");
    if (accessToken.empty()) {
        ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                .setMessage(i18n("invalidArguments"))
                .to(failedCb);
        return;
    }
    try {
        const auto [playerId, newAccessToken] = app().getPlugin<PlayerManager>()->getPlayerIdByAccessToken(accessToken);
        if (newAccessToken.has_value()) {
            req->attributes()->insert("accessToken", newAccessToken.value());
        }
        req->attributes()->insert("playerId", playerId);
    } catch (const ResponseException &e) {
        e.toJson().to(failedCb);
        return;
    }
    nextCb();
}


