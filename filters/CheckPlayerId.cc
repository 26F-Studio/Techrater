//
// Created by particleg on 2021/9/27.
//

#include <filters/CheckPlayerId.h>

using namespace drogon;
using namespace std;
using namespace techmino::filters;
using namespace techmino::helpers;

void CheckPlayerId::doFilter(
        const HttpRequestPtr &req,
        FilterCallback &&failedCb,
        FilterChainCallback &&nextCb
) {
    const auto &playerId = req->getParameter("playerId");
    req->attributes()->insert("playerId", playerId.empty() ? -1 : stol(playerId));
    nextCb();
}


