//
// Created by particleg on 2021/9/24.
//

#include <controllers/Admin.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <magic_enum.hpp>
#include <types/Action.h>
#include <utils/datetime.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;

Admin::Admin() : _connectionManager(app().getPlugin<ConnectionManager>()) {}

void Admin::shutdownAfter(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        const auto delay = req->attributes()->get<RequestJson>("requestJson")["delay"].asDouble();

        Json::Value data;
        data["message"] = i18n("seconds");
        data["data"] = delay;
        _connectionManager->broadcast(
                MessageJson(enum_integer(Action::GlobalNotification)).setData(std::move(data))
        );

        app().getLoop()->runAfter(delay, []() {
            app().quit();
        });

        response.setData(datetime::toString(
                datetime::toDate().after(delay)
        ));
    }, response);
    response.to(callback);
}