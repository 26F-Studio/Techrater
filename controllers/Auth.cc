//
// Created by particleg on 2021/9/24.
//

#include <controllers/Auth.h>
#include <helpers/ResponseJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::types;

Auth::Auth() : _playerManager(app().getPlugin<PlayerManager>()) {}

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

void Auth::oauth(const HttpRequestPtr &req, function<void(const drogon::HttpResponsePtr &)> &&callback, string &&token) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->oauth(
                req->attributes()->get<RequestJson>("requestJson")["playerId"].asInt64(),
                token
        ));
    }, response);
    response.to(callback);
}
