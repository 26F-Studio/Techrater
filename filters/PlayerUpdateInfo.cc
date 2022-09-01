//
// Created by particleg on 2021/9/27.
//

#include <filters/PlayerUpdateInfo.h>
#include <helpers/RequestJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::types;

void PlayerUpdateInfo::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        auto request = RequestJson(req);
        request.remove("id");
        request.remove("avatar_hash");
        request.remove("permission");
        request.remove("password_hash");
        request.remove("email");
        request.remove("phone");
        request.trim("username", JsonValue::String);
        request.trim("motto", JsonValue::String);
        request.trim("region", JsonValue::Int64);
        request.trim("avatar", JsonValue::String);
        request.trim("avatar_frame", JsonValue::Int64);
        request.trim("clan", JsonValue::Int64);
        request.trim("password", JsonValue::String);
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
