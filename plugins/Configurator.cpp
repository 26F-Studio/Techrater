//
// Created by particleg on 12/6/22.
//

#include <drogon/drogon.h>
#include <magic_enum.hpp>
#include <plugins/Configurator.h>
#include <types/ResultCode.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::types;

void Configurator::initAndStart(const Json::Value &config) {
    if (config["block"]["enable"].isBool() && config["block"]["enable"].asBool()) {
        app().registerSyncAdvice([=](const HttpRequestPtr &req) -> HttpResponsePtr {
            LOG_DEBUG << "Sync advice: " << req->path();
            for (const auto &whitePath: config["block"]["whiteList"]) {
                if (req->path() == whitePath.asString()) {
                    return nullptr;
                }
            }

            Json::Value body;
            body["code"] = enum_integer(ResultCode::NotAvailable);
            body["data"] = "苟利国家生死以，岂因祸福避趋之";
            body["message"] = "Mourn for him.";
            body["reason"] = "Mourn for him.";

            auto resp = HttpResponse::newHttpJsonResponse(body);
            resp->setStatusCode(k410Gone);

            return resp;
        });
    }
    LOG_INFO << "Configurator loaded.";
}

void Configurator::shutdown() { LOG_INFO << "Configurator shutdown."; }