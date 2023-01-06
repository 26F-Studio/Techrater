//
// Created by particleg on 2021/9/27.
//

#include <filters/AuthOauth.h>
#include <helpers/RequestJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::types;

void AuthOauth::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        if (!req->peerAddr().isIntranetIp()) {
            auto response = HttpResponse::newNotFoundResponse();
            failedCb(response);
            return;
        }
        auto request = RequestJson(req);
        request.require("playerId", JsonValue::Int64);
        request.require("accessToken", JsonValue::String);
        request.require("expiration", JsonValue::UInt64);
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
