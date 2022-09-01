//
// Created by particleg on 2021/9/27.
//

#include <filters/AuthSeedEmail.h>
#include <helpers/RequestJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::types;

void AuthSeedEmail::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        auto request = RequestJson(req);
        request.require("email", JsonValue::String);
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
