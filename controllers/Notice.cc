//
// Created by particleg on 2022/9/1.
//

#include <controllers/Notice.h>
#include <helpers/ResponseJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

Notice::Notice() : _noticeManager(app().getPlugin<NoticeManager>()) {}

void Notice::createNotice(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_noticeManager->createNotice(
                req->attributes()->get<Json::Value>("contents")
        ));
    }, response);
    response.to(callback);
}

void Notice::retrieveNotice(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        const auto lastCount = req->attributes()->get<uint64_t>("lastCount");
        if (lastCount) {
            response.setData(_noticeManager->retrieveNotice(
                    lastCount,
                    req->attributes()->get<string>("language")
            ));
        } else {
            response.setData(_noticeManager->retrieveNotice(
                    req->attributes()->get<uint64_t>("pageIndex"),
                    req->attributes()->get<uint64_t>("pageSize"),
                    req->attributes()->get<string>("language")
            ));
        }
    }, response);
    response.to(callback);
}

void Notice::updateNotice(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        _noticeManager->updateNotice(
                req->attributes()->get<uint64_t>("noticeId"),
                req->attributes()->get<Json::Value>("requestJson")
        );
    }, response);
    response.to(callback);
}

void Notice::deleteNotice(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        _noticeManager->deleteNotice(
                req->attributes()->get<uint64_t>("noticeId")
        );
    }, response);
    response.to(callback);
}
