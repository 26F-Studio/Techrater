//
// Created by particleg on 2021/9/24.
//

#include <controllers/Auth.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

Auth::Auth() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Auth::oauth(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto responseJson = req->attributes()->get<ResponseJson>("responseJson");
        _playerManager->oauth(
                responseJson["playerId"].asInt64(),
                responseJson["accessToken"].asString(),
                chrono::milliseconds(responseJson["expiration"].asUInt64())
        );
    }, response);
    response.to(callback);
}

void Auth::check(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(req->attributes()->get<int64_t>("playerId"));
    }, response);
    response.to(callback);
}
