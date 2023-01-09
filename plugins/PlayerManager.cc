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

    if (!(
            config["auth"]["host"].isString()
    )) {
        LOG_ERROR << R"(Invalid auth config)";
        abort();
    }
    _authAddress = config["auth"]["host"].asString();

    LOG_INFO << "PlayerManager loaded.";
}

void PlayerManager::shutdown() {
    LOG_INFO << "PlayerManager shutdown.";
}

pair<int64_t, optional<string>> PlayerManager::getPlayerIdByAccessToken(const string &accessToken) {
    setClient(_authAddress);
    const auto response = request(
            Get,
            "/auth/check",
            {
                    {"code", JsonValue::UInt64},
                    {"data", JsonValue::Object},
            },
            {{"x-access-token", accessToken}},
            Json::nullValue,
            false
    );
    if (response["code"].asUInt64() / 100 != 2) {
        throw ResponseException(
                response["message"].isString() ? response["message"].asString() : i18n("invalidResponse"),
                internal::BaseException(
                        response["reason"].isString() ? response["reason"].asString() : "Unknown error"
                ),
                enum_cast<ResultCode>(response["code"].asUInt64()).value_or(ResultCode::Unknown),
                k406NotAcceptable
        );
    }
    return {
            response["playerId"].asInt64(),
            response["code"].asUInt64() == enum_integer(ResultCode::Continued) ?
            optional<string>{response["accessToken"].asString()} :
            optional<string>{nullopt}
    };
}

Json::Value PlayerManager::getPlayerData(
        const string &accessToken,
        int64_t playerId
) {
    int64_t targetId = playerId;
    NO_EXCEPTION(
            targetId = getPlayerIdByAccessToken(accessToken).first;
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
