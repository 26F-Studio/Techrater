//
// Created by particleg on 2021/9/27.
//

#include <filters/IdentifyUser.h>
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

void IdentifyUser::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    auto accessToken = req->attributes()->get<string>("accessToken");
    if (accessToken.empty()) {
        ResponseJson response;
        response.setStatusCode(k400BadRequest);
        response.setResultCode(ResultCode::InvalidArguments);
        response.setMessage(i18n("invalidArguments"));
        response.httpCallback(failedCb);
        return;
    }
    try {
        int64_t id = app().getPlugin<PlayerManager>()->getUserId(accessToken);
        req->attributes()->insert("id", id);
    } catch (const ResponseException &e) {
        ResponseJson response;
        response.setStatusCode(e.statusCode());
        response(e.toJson());
        response.httpCallback(failedCb);
        return;
    }
    nextCb();
}


