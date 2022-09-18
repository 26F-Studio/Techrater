//
// Created by particleg on 2021/9/27.
//

#include <filters/PlayerUpdateData.h>
#include <helpers/RequestJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::types;

void PlayerUpdateData::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&]() {
        auto request = RequestJson(req);
        request.remove("id");
        request.trim("statistics", JsonValue::String);
        request.trim("ranks", JsonValue::String);
        request.trim("settings", JsonValue::String);
        request.trim("keymaps", JsonValue::String);
        request.trim("touch_1", JsonValue::String);
        request.trim("touch_2", JsonValue::String);
        request.trim("touch_3", JsonValue::String);
        request.trim("extra_1", JsonValue::String);
        request.trim("extra_2", JsonValue::String);
        request.trim("extra_3", JsonValue::String);
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
