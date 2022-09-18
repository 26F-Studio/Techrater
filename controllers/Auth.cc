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

void Auth::check(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(req->attributes()->get<int64_t>("playerId"));
    }, response);
    response.httpCallback(callback);
}

void Auth::refresh(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->refresh(
                req->attributes()->get<string>("refreshToken")
        ).parse());
    }, response);
    response.httpCallback(callback);
}

void Auth::verifyEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        _playerManager->verifyEmail(
                req->attributes()->get<RequestJson>("requestJson")["email"].asString(),
                callback
        );
    }, response);
}

void Auth::seedEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->seedEmail(
                req->attributes()->get<RequestJson>("requestJson")["email"].asString()
        ));
    }, response);
    response.httpCallback(callback);
}

void Auth::loginEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        if (request.check("code", JsonValue::String)) {
            const auto &[tokens, isNew] = _playerManager->loginEmailCode(
                    request["email"].asString(),
                    request["code"].asString()
            );
            if (isNew) { response.setResultCode(ResultCode::Continued); }
            response.setData(tokens.parse());
        } else {
            const auto &tokens = _playerManager->loginEmailPassword(
                    request["email"].asString(),
                    request["password"].asString()
            );
            response.setData(tokens.parse());
        }
    }, response);
    response.httpCallback(callback);
}

void Auth::resetEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->resetEmail(
                request["email"].asString(),
                request["code"].asString(),
                request["newPassword"].asString()
        );
    }, response);
    response.httpCallback(callback);
}

void Auth::migrateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->migrateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["newEmail"].asString(),
                request["code"].asString()
        );
    }, response);
    response.httpCallback(callback);
}

void Auth::deactivateEmail(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        auto request = req->attributes()->get<RequestJson>("requestJson");
        _playerManager->deactivateEmail(
                req->attributes()->get<int64_t>("playerId"),
                request["code"].asString()
        );
    }, response);
    response.httpCallback(callback);
}