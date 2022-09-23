//
// Created by particleg on 2022/8/29.
//

#include <drogon/drogon.h>
#include <plugins/EmailManager.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>
#include <structures/Exceptions.h>
#include <utils/crypto.h>
#include <utils/data.h>
#include <utils/io.h>

using namespace drogon;
using namespace drogon_model;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;

PlayerManager::PlayerManager() :
        _dataMapper(app().getDbClient()),
        _playerMapper(app().getDbClient()) {}

void PlayerManager::initAndStart(const Json::Value &config) {
    if (!(
            config["tokenBucket"]["ip"]["interval"].isUInt64() &&
            config["tokenBucket"]["ip"]["maxCount"].isUInt64() &&
            config["tokenBucket"]["email"]["interval"].isUInt64() &&
            config["tokenBucket"]["email"]["maxCount"].isUInt64()
    )) {
        LOG_ERROR << R"(Invalid tokenBucket config)";
        abort();
    }
    _ipInterval = chrono::seconds(config["tokenBucket"]["ip"]["interval"].asUInt64());
    _ipMaxCount = config["tokenBucket"]["ip"]["maxCount"].asUInt64();
    _emailInterval = chrono::seconds(config["tokenBucket"]["email"]["interval"].asUInt64());
    _emailMaxCount = config["tokenBucket"]["email"]["maxCount"].asUInt64();

    if (!(
            config["redis"]["host"].isString() &&
            config["redis"]["port"].isUInt() &&
            config["redis"]["db"].isInt() &&
            config["redis"]["timeout"].isUInt() &&
            config["redis"]["expirations"]["refresh"].isInt() &&
            config["redis"]["expirations"]["access"].isInt() &&
            config["redis"]["expirations"]["email"].isInt()
    )) {
        LOG_ERROR << R"("Invalid redis config")";
        abort();
    }

    _playerRedis = make_unique<PlayerRedis>(std::move(PlayerRedis(
            {
                    chrono::minutes(config["redis"]["expirations"]["refresh"].asInt64()),
                    chrono::minutes(config["redis"]["expirations"]["access"].asInt64()),
                    chrono::minutes(config["redis"]["expirations"]["email"].asInt64())
            }
    )));

    try {
        _playerRedis->connect(
                config["redis"]["host"].asString(),
                config["redis"]["port"].asUInt(),
                config["redis"]["db"].asInt(),
                config["redis"]["timeout"].asUInt()
        );
    } catch (const cpp_redis::redis_error &e) {
        LOG_ERROR << e.what();
        abort();
    }
    LOG_INFO << "PlayerManager loaded.";
}

void PlayerManager::shutdown() {
    _playerRedis->disconnect();
    LOG_INFO << "PlayerManager shutdown.";
}

int64_t PlayerManager::getPlayerId(const string &accessToken) {
    try {
        return _playerRedis->getIdByAccessToken(accessToken);
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidAccessToken"),
                e,
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

RedisToken PlayerManager::refresh(const string &refreshToken) {
    try {
        return std::move(_playerRedis->refresh(refreshToken));
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidRefreshToken"),
                e,
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

void PlayerManager::verifyEmail(
        const string &email,
        const function<void(const HttpResponsePtr &)> &callback
) {
    auto code = data::randomString(8);
    _playerRedis->setEmailCode(email, code);
    auto mailContent = io::getFileContent("./verifyEmail.html");
    drogon::utils::replaceAll(
            mailContent,
            "{{VERIFY_CODE}}",
            code
    );
    // TODO: Replace with future
    app().getPlugin<EmailManager>()->smtp(
            email,
            "[techrater] Verify Code 验证码",
            mailContent,
            true,
            [callback, this](bool result, const string &receivedMessage) {
                if (result) {
                    ResponseJson().to(callback);
                } else {
                    ResponseJson(k500InternalServerError, ResultCode::EmailError)
                            .setMessage(i18n("emailSendError"))
                            .setReason(receivedMessage)
                            .to(callback);
                }
            }
    );
}

string PlayerManager::seedEmail(const string &email) {
    try {
        const auto player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));
        return crypto::blake2B(to_string(player.getValueOfId()));
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("invalidEmail"),
                ResultCode::NotAcceptable,
                k403Forbidden
        );
    }
}

tuple<RedisToken, bool> PlayerManager::loginEmailCode(const string &email, const string &code) {
    _checkEmailCode(email, code);

    techrater::Player player;
    if (_playerMapper.count(orm::Criteria(
            techrater::Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
    )) == 0) {
        player.setEmail(email);
        _playerMapper.insert(player);

        techrater::Data data;
        data.setId(player.getValueOfId());
        _dataMapper.insert(data);
    } else {
        player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));
    }

    return {
            _playerRedis->generateTokens(to_string(player.getValueOfId())),
            player.getPasswordHash() == nullptr
    };
}

RedisToken PlayerManager::loginEmailPassword(const string &email, const string &password) {
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));

        if (player.getPasswordHash() == nullptr) {
            throw ResponseException(
                    i18n("noPassword"),
                    ResultCode::NullValue,
                    k403Forbidden
            );
        }

        const auto id = to_string(player.getValueOfId());

        if (player.getValueOfPasswordHash() != crypto::blake2B(password + crypto::blake2B(id))) {
            throw orm::UnexpectedRows("Incorrect password");
        }

        return _playerRedis->generateTokens(id);
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("invalidEmailPass"),
                ResultCode::NotAcceptable,
                k403Forbidden
        );
    }
}

void PlayerManager::resetEmail(
        const string &email,
        const string &code,
        const string &newPassword
) {
    _checkEmailCode(email, code);

    try {
        auto player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));
        player.setPasswordHash(crypto::blake2B(
                newPassword + crypto::blake2B(to_string(player.getValueOfId()))
        ));
        _playerMapper.update(player);
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("userNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::migrateEmail(
        const int64_t userId,
        const string &newEmail,
        const string &code
) {
    _checkEmailCode(newEmail, code);

    try {
        auto player = _playerMapper.findByPrimaryKey(userId);
        if (player.getValueOfEmail() == newEmail) {
            return;
        }
        if (_playerMapper.count(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                newEmail
        ))) {
            throw ResponseException(
                    i18n("emailExists"),
                    ResultCode::Conflict,
                    k409Conflict
            );
        }
        player.setEmail(newEmail);
        _playerMapper.update(player);
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidAccessToken"),
                e,
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}


void PlayerManager::deactivateEmail(
        const int64_t userId,
        const string &code
) {
    try {
        auto player = _playerMapper.findByPrimaryKey(userId);
        _checkEmailCode(player.getValueOfEmail(), code);

        _playerMapper.deleteOne(player);
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidAccessToken"),
                e,
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

string PlayerManager::getAvatar(const string &accessToken, int64_t userId) {
    int64_t targetId = userId;
    NO_EXCEPTION(
            targetId = _playerRedis->getIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_id,
                orm::CompareOperator::EQ,
                targetId
        ));
        return player.getValueOfAvatar();
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("userNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

Json::Value PlayerManager::getPlayerInfo(
        const string &accessToken,
        int64_t userId
) {
    int64_t targetId = userId;
    NO_EXCEPTION(
            targetId = _playerRedis->getIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper.findOne(orm::Criteria(
                techrater::Player::Cols::_id,
                orm::CompareOperator::EQ,
                targetId
        )).toJson();
        player.removeMember("password_hash");
        player.removeMember("avatar");
        if (userId > 0) {
            player.removeMember("email");
            player.removeMember("phone");
        }
        return player;
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("userNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::updatePlayerInfo(
        int64_t userId,
        RequestJson request
) {
    auto player = _playerMapper.findByPrimaryKey(userId);
    if (player.getPasswordHash() == nullptr) {
        if (!request.check("password", JsonValue::String)) {
            throw ResponseException(
                    i18n("noPassword"),
                    ResultCode::NullValue,
                    k403Forbidden
            );
        }
        player.setPasswordHash(crypto::blake2B(
                request["password"].asString() + crypto::blake2B(to_string(player.getValueOfId()))
        ));
    }
    if (request.check("avatar", JsonValue::String)) {
        player.setAvatarHash(crypto::blake2B(request["avatar"].asString()));
    }
    player.updateByJson(request.ref());
    _playerMapper.update(player);
}

Json::Value PlayerManager::getPlayerData(
        const string &accessToken,
        int64_t userId
) {
    int64_t targetId = userId;
    NO_EXCEPTION(
            targetId = _playerRedis->getIdByAccessToken(accessToken);
    )
    try {
        auto data = _dataMapper.findOne(orm::Criteria(
                techrater::Data::Cols::_id,
                orm::CompareOperator::EQ,
                targetId
        )).toJson();
        if (userId > 0) {
            data.removeMember("settings");
            data.removeMember("keymaps");
            data.removeMember("touch_1");
            data.removeMember("touch_2");
            data.removeMember("touch_3");
        }
        return data;
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("userNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::updatePlayerData(
        int64_t userId,
        RequestJson request
) {
    auto data = _dataMapper.findByPrimaryKey(userId);
    data.updateByJson(request.ref());
    _dataMapper.update(data);
}

bool PlayerManager::ipLimit(const string &ip) const {
    return _playerRedis->tokenBucket(
            "ip:" + ip,
            _ipInterval,
            _ipMaxCount
    );
}

bool PlayerManager::emailLimit(const string &email) const {
    return _playerRedis->tokenBucket(
            "email:" + email,
            _emailInterval,
            _emailMaxCount
    );
}

void PlayerManager::_checkEmailCode(const string &email, const string &code) {
    try {
        if (!_playerRedis->checkEmailCode(email, code)) {
            throw ResponseException(
                    i18n("invalidCode"),
                    ResultCode::NotAcceptable,
                    k403Forbidden
            );
        }
        _playerRedis->deleteEmailCode(email);
    } catch (const redis_exception::KeyNotFound &e) {
        throw ResponseException(
                i18n("invalidEmail"),
                e,
                ResultCode::NotFound,
                k404NotFound
        );
    }
}
