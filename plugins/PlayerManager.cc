//
// Created by particleg on 2022/8/29.
//

#include <drogon/drogon.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>
#include <structures/Exceptions.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace drogon_model;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;

PlayerManager::PlayerManager() :
        RedisHelper("player"),
        _dataMapper(app().getDbClient()),
        _playerMapper(app().getDbClient()) {}

void PlayerManager::initAndStart(const Json::Value &config) {
    if (!(
            config["tokenBucket"]["ip"]["interval"].isUInt64() &&
            config["tokenBucket"]["ip"]["maxCount"].isUInt64() &&
            config["tokenBucket"]["login"]["interval"].isUInt64() &&
            config["tokenBucket"]["login"]["maxCount"].isUInt64() &&
            config["tokenBucket"]["verify"]["interval"].isUInt64() &&
            config["tokenBucket"]["verify"]["maxCount"].isUInt64()
    )) {
        LOG_ERROR << R"(Invalid tokenBucket config)";
        abort();
    }
    _ipInterval = chrono::seconds(config["tokenBucket"]["ip"]["interval"].asUInt64());
    _ipMaxCount = config["tokenBucket"]["ip"]["maxCount"].asUInt64();
    _loginInterval = chrono::seconds(config["tokenBucket"]["login"]["interval"].asUInt64());
    _loginMaxCount = config["tokenBucket"]["login"]["maxCount"].asUInt64();
    _verifyInterval = chrono::seconds(config["tokenBucket"]["verify"]["interval"].asUInt64());
    _verifyMaxCount = config["tokenBucket"]["verify"]["maxCount"].asUInt64();

    LOG_INFO << "PlayerManager loaded.";
}

void PlayerManager::shutdown() {
    LOG_INFO << "PlayerManager shutdown.";
}

void PlayerManager::oauth(int64_t playerId, const string &accessToken, chrono::milliseconds expiration) {
    setPx("auth:access-id:" + accessToken, to_string(playerId), expiration);
}

int64_t PlayerManager::getPlayerIdByAccessToken(const string &accessToken) {
    try {
        return stoll(get("auth:access-id:" + accessToken));
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidAccessToken"),
                e,
                ResultCode::NotFound,
                drogon::k404NotFound
        );
    }
}

Json::Value PlayerManager::getPlayerData(
        const string &accessToken,
        int64_t playerId
) {
    int64_t targetId = playerId;
    NO_EXCEPTION(
            targetId = getPlayerIdByAccessToken(accessToken);
    )
    try {
        auto data = _dataMapper.findOne(orm::Criteria(
                techrater::Data::Cols::_id,
                orm::CompareOperator::EQ,
                targetId
        )).toJson();
        if (playerId > 0) {
            data.removeMember("settings");
            data.removeMember("keymaps");
            data.removeMember("touch_1");
            data.removeMember("touch_2");
            data.removeMember("touch_3");
        }
        return data;
    } catch (const orm::UnexpectedRows &) {
        LOG_DEBUG << "playerNotFound: " << targetId;
        throw ResponseException(
                i18n("playerNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::updatePlayerData(
        int64_t playerId,
        RequestJson request
) {
    auto data = _dataMapper.findByPrimaryKey(playerId);
    data.updateByJson(request.ref());
    _dataMapper.update(data);
}

bool PlayerManager::ipLimit(const string &ip) {
    return tokenBucket(
            "ip:" + ip,
            _ipInterval,
            _ipMaxCount
    );
}

bool PlayerManager::loginLimit(const string &type, const string &key) {
    return tokenBucket(
            "code:" + type + ":" + key,
            _loginInterval,
            _loginMaxCount
    );
}

bool PlayerManager::verifyLimit(const string &type, const string &key) {
    return tokenBucket(
            "verify:" + type + ":" + key,
            _verifyInterval,
            _verifyMaxCount
    );
}
