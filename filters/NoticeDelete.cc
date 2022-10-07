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
    const auto &noticeId = req->getParameter("noticeId");
    req->attributes()->insert("noticeId", noticeId.empty() ? -1 : stoll(noticeId));
    nextCb();
}
