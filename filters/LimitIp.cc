//
// Created by particleg on 2021/9/27.
//

#include <filters/LimitIp.h>
#include <helpers/ResponseJson.h>
#include <plugins/PlayerManager.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

static string getClientIp(
    const HttpRequestPtr& req
) {
    auto& xForwardedFor = req->getHeader("X-Forwarded-For");
    if (!xForwardedFor.empty()) {
        return xForwardedFor.substr(0, xForwardedFor.find(','));
    }
    auto& xRealIp = req->getHeader("X-Real-IP");
    if (!xRealIp.empty()) {
        return xRealIp;
    }
	return req->getPeerAddr().toIp();
}

void LimitIp::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    try {
        if (!app().getPlugin<PlayerManager>()->ipLimit(getClientIp(req))) {
            ResponseJson(k429TooManyRequests, ResultCode::TooFrequent)
                    .setMessage(i18n("tooFrequent"))
                    .to(failedCb);
            return;
        }
    } catch (const exception &e) {
        LOG_ERROR << e.what();
        ResponseJson(k500InternalServerError, ResultCode::InternalError)
                .setMessage(i18n("internalError"))
                .to(failedCb);
        return;
    }
    nextCb();
}
