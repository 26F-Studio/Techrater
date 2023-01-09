//
// Created by particleg on 2021/9/24.
//

#include <controllers/Auth.h>
#include <helpers/ResponseJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::types;

Auth::Auth() = default;

void Auth::check(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        Json::Value data;
        data["playerId"] = req->attributes()->get<int64_t>("playerId");
        const auto accessToken = req->attributes()->get<string>("accessToken");
        if (!accessToken.empty()) {
            data["accessToken"] = accessToken;
            response.setResultCode(ResultCode::Continued);
        }
        response.setData(data);
    }, response);
    response.to(callback);
}
