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
    const auto lastCount = req->getParameter("lastCount");
    const auto pageIndex = req->getParameter("pageIndex");
    const auto pageSize = req->getParameter("pageSize");
    const auto language = req->getParameter("language");

    if (language.empty()) {
        ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                .setMessage(i18n("invalidArguments"))
                .to(failedCb);
        return;
    }
    req->attributes()->insert("language", language);

    if (lastCount.empty()) {
        if (pageIndex.empty() || pageSize.empty()) {
            ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                    .setMessage(i18n("invalidArguments"))
                    .to(failedCb);
            return;
        }
        req->attributes()->insert("pageIndex", pageIndex);
        req->attributes()->insert("pageSize", pageSize);
    } else {
        req->attributes()->insert("lastCount", lastCount);
    }
    nextCb();
}


