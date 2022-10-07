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
        request.remove("create_time");
        request.trim("en_us", JsonValue::String);
        request.trim("zh_cn", JsonValue::String);
        request.trim("zh_tw", JsonValue::String);
        request.trim("fr_fr", JsonValue::String);
        request.trim("es_es", JsonValue::String);
        request.trim("pt_pt", JsonValue::String);
        request.trim("id_id", JsonValue::String);
        request.trim("ja_jp", JsonValue::String);
        req->attributes()->insert("contents", request.ref());
        nextCb();
    }, failedCb);
}
