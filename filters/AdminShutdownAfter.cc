//
// Created by particleg on 2021/9/27.
//

#include <filters/AdminShutdownAfter.h>
#include <helpers/RequestJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::types;

void AdminShutdownAfter::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    handleExceptions([&] {
        auto request = RequestJson(req);
        request.require("delay", JsonValue::Double);
        if (request["delay"].asDouble() < 0) {
            ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                    .setMessage(i18n("invalidArguments"))
                    .to(failedCb);
            return;
        }
        req->attributes()->insert("requestJson", request);
        nextCb();
    }, failedCb);
}
