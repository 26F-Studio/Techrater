//
// Created by particleg on 2021/9/24.
//

#include <controllers/Admin.h>
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

Admin::Admin() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Admin::shutdown(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(req->attributes()->get<int64_t>("playerId"));
    }, response);
    response.to(callback);
}