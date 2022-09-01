//
// Created by particleg on 2022/9/1.
//

#include <controllers/Player.h>
#include <helpers/ResponseJson.h>

using namespace drogon;
using namespace std;
using namespace techmino::api::v1;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

Player::Player() : _playerManager(app().getPlugin<PlayerManager>()) {}

void Player::getInfo(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->getUserInfo(
                req->attributes()->get<string>("accessToken"),
                req->attributes()->get<int64_t>("playerId")
        ));
    }, response);
    response.httpCallback(callback);
}

void Player::updateInfo(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        _playerManager->updateUserInfo(
                req->attributes()->get<int64_t>("playerId"),
                req->attributes()->get<RequestJson>("requestJson")
        );
    }, response);
    response.httpCallback(callback);
}

void Player::getAvatar(const HttpRequestPtr &req, function<void(const HttpResponsePtr &)> &&callback) {
    ResponseJson response;
    handleExceptions([&]() {
        response.setData(_playerManager->getAvatar(
                req->attributes()->get<string>("accessToken"),
                req->attributes()->get<int64_t>("playerId")
        ));
    }, response);
    response.httpCallback(callback);
}