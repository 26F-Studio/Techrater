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
        ResponseJson response;
        response.setStatusCode(k400BadRequest);
        response.setResultCode(ResultCode::InvalidArguments);
        response.setMessage(i18n("invalidArguments"));
        response.httpCallback(failedCb);
        return;
    }
    try {
        req->attributes()->insert(
                "playerId",
                app().getPlugin<PlayerManager>()->getPlayerId(accessToken)
        );
    } catch (const ResponseException &e) {
        ResponseJson response;
        response.setStatusCode(e.statusCode());
        response(e.toJson());
        response.httpCallback(failedCb);
        return;
    }
    nextCb();
}


