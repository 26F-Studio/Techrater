//
// Created by particleg on 2021/9/27.
//

#include <filters/LimitEmail.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <plugins/PlayerManager.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

void LimitEmail::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    auto requestJson = req->attributes()->get<RequestJson>("requestJson");
    try {
        if (!app().getPlugin<PlayerManager>()->emailLimit(requestJson["email"].asString())) {
            ResponseJson response;
            response.setStatusCode(k429TooManyRequests);
            response.setResultCode(ResultCode::TooFrequent);
            response.setMessage(i18n("tooFrequent"));
            response.httpCallback(failedCb);
            return;
        }
    } catch (const exception &e) {
        LOG_ERROR << e.what();
        ResponseJson response;
        response.setStatusCode(k500InternalServerError);
        response.setResultCode(ResultCode::InternalError);
        response.setMessage(i18n("internalError"));
        response.httpCallback(failedCb);
        return;
    }
    nextCb();
}
