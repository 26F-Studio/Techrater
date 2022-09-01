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
#include <utils/datetime.h>
#include <utils/io.h>

using namespace drogon;
using namespace drogon_model;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;
using namespace techmino::utils;

void PlayerManager::initAndStart(const Json::Value &config) {
    if (!(
            config["tokenBucket"]["ip"]["interval"].isUInt64() &&
            config["tokenBucket"]["ip"]["maxCount"].isUInt64() &&
            config["tokenBucket"]["email"]["interval"].isUInt64() &&
            config["tokenBucket"]["email"]["maxCount"].isUInt64() &&
            config["tokenBucket"]["phone"]["interval"].isUInt64() &&
            config["tokenBucket"]["phone"]["maxCount"].isUInt64()
    )) {
        LOG_ERROR << R"(Invalid tokenBucket config)";
        abort();
    }
    _ipInterval = chrono::seconds(config["tokenBucket"]["ip"]["interval"].asUInt64());
    _ipMaxCount = config["tokenBucket"]["ip"]["maxCount"].asUInt64();
    _emailInterval = chrono::seconds(config["tokenBucket"]["email"]["interval"].asUInt64());
    _emailMaxCount = config["tokenBucket"]["email"]["maxCount"].asUInt64();
    _phoneInterval = chrono::seconds(config["tokenBucket"]["phone"]["interval"].asUInt64());
    _phoneMaxCount = config["tokenBucket"]["phone"]["maxCount"].asUInt64();

    if (!(
            config["redis"]["host"].isString() &&
            config["redis"]["port"].isUInt() &&
            config["redis"]["db"].isInt() &&
            config["redis"]["timeout"].isUInt() &&
            config["redis"]["expirations"]["refresh"].isInt() &&
            config["redis"]["expirations"]["access"].isInt() &&
            config["redis"]["expirations"]["email"].isInt() &&
            config["redis"]["expirations"]["phone"].isInt()
    )) {
        LOG_ERROR << R"("Invalid redis config")";
        abort();
    }

    _userRedis = make_unique<PlayerRedis>(std::move(PlayerRedis(
            {
                    chrono::minutes(config["redis"]["expirations"]["refresh"].asInt64()),
                    chrono::minutes(config["redis"]["expirations"]["access"].asInt64()),
                    chrono::minutes(config["redis"]["expirations"]["email"].asInt64()),
                    chrono::minutes(config["redis"]["expirations"]["phone"].asInt64())
            }
    )));

    try {
        _userRedis->connect(
                config["redis"]["host"].asString(),
                config["redis"]["port"].asUInt(),
                config["redis"]["db"].asInt(),
                config["redis"]["timeout"].asUInt()
        );
    } catch (const cpp_redis::redis_error &e) {
        LOG_ERROR << e.what();
        abort();
    }

    _playerMapper = make_unique<orm::Mapper<techrater::Player>>(app().getDbClient());

    LOG_INFO << "PlayerManager loaded.";
}

void PlayerManager::shutdown() {
    _userRedis->disconnect();
    LOG_INFO << "PlayerManager shutdown.";
}

int64_t PlayerManager::getPlayerId(const string &accessToken) {
    try {
        return _userRedis->getIdByAccessToken(accessToken);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidAccessToken"),
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

RedisToken PlayerManager::refresh(const string &refreshToken) {
    try {
        return std::move(_userRedis->refresh(refreshToken));
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidRefreshToken"),
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

void PlayerManager::verifyEmail(const string &email) {
    auto code = data::randomString(8);
    _userRedis->setEmailCode(email, code);
    auto mailContent = io::getFileContent("./verifyEmail.html");
    drogon::utils::replaceAll(
            mailContent,
            "{{VERIFY_CODE}}",
            code
    );
    // TODO: Replace with async method
    app().getPlugin<EmailManager>()->smtp(
            email,
            "[techrater] Verify Code 验证码",
            mailContent,
            true,
            {}
    );
}

void PlayerManager::verifyPhone(const string &phone) {
    auto code = data::randomString(8);
    _userRedis->setPhoneCode(phone, code);
    throw ResponseException(
            i18n("notImplemented"),
            ResultCode::InternalError,
            k501NotImplemented
    );
}

string PlayerManager::seedEmail(const string &email) {
    try {
        const auto player = _playerMapper->findOne(orm::Criteria(
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

string PlayerManager::seedPhone(const string &phone) {
    try {
        const auto player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_phone,
                orm::CompareOperator::EQ,
                phone
        ));
        return crypto::blake2B(to_string(player.getValueOfId()));
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("invalidPhone"),
                ResultCode::NotAcceptable,
                k403Forbidden
        );
    }
}

tuple<RedisToken, bool> PlayerManager::loginEmailCode(const string &email, const string &code) {
    _checkEmailCode(email, code);

    techrater::Player player;
    if (_playerMapper->count(orm::Criteria(
            techrater::Player::Cols::_email,
            orm::CompareOperator::EQ,
            email
    )) == 0) {
        player.setEmail(email);
        _playerMapper->insert(player);
    } else {
        player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));
    }

    return {
            _userRedis->generateTokens(to_string(player.getValueOfId())),
            player.getPasswordHash() == nullptr
    };
}

tuple<RedisToken, bool> PlayerManager::loginPhoneCode(const string &phone, const string &code) {
    _checkPhoneCode(phone, code);

    techrater::Player player;
    if (_playerMapper->count(orm::Criteria(
            techrater::Player::Cols::_phone,
            orm::CompareOperator::EQ,
            phone
    )) == 0) {
        player.setPhone(phone);
        _playerMapper->insert(player);
    } else {
        player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_phone,
                orm::CompareOperator::EQ,
                phone
        ));
    }

    return {
            _userRedis->generateTokens(to_string(player.getValueOfId())),
            player.getPasswordHash() == nullptr
    };
}

RedisToken PlayerManager::loginEmailPassword(const string &email, const string &password) {
    try {
        auto player = _playerMapper->findOne(orm::Criteria(
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

        return _userRedis->generateTokens(id);
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("invalidEmailPass"),
                ResultCode::NotAcceptable,
                k403Forbidden
        );
    }
}

RedisToken PlayerManager::loginPhonePassword(const string &phone, const string &password) {
    try {
        auto player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_phone,
                orm::CompareOperator::EQ,
                phone
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

        return _userRedis->generateTokens(id);
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
        auto player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_email,
                orm::CompareOperator::EQ,
                email
        ));
        player.setPasswordHash(crypto::blake2B(
                newPassword + crypto::blake2B(to_string(player.getValueOfId()))
        ));
        _playerMapper->update(player);
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("userNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::resetPhone(
        const string &phone,
        const string &code,
        const string &newPassword
) {
    _checkPhoneCode(phone, code);

    try {
        auto player = _playerMapper->findOne(orm::Criteria(
                techrater::Player::Cols::_phone,
                orm::CompareOperator::EQ,
                phone
        ));
        player.setPasswordHash(crypto::blake2B(
                newPassword + crypto::blake2B(to_string(player.getValueOfId()))
        ));
        _playerMapper->update(player);
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
        auto player = _playerMapper->findByPrimaryKey(userId);
        if (player.getValueOfEmail() == newEmail) {
            return;
        }
        if (_playerMapper->count(orm::Criteria(
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
        _playerMapper->update(player);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidAccessToken"),
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

void PlayerManager::migratePhone(
        const int64_t userId,
        const string &newPhone,
        const string &code
) {
    _checkPhoneCode(newPhone, code);

    try {
        auto player = _playerMapper->findByPrimaryKey(userId);
        if (player.getValueOfPhone() == newPhone) {
            return;
        }
        if (_playerMapper->count(orm::Criteria(
                techrater::Player::Cols::_phone,
                orm::CompareOperator::EQ,
                newPhone
        ))) {
            throw ResponseException(
                    i18n("emailExists"),
                    ResultCode::Conflict,
                    k409Conflict
            );
        }
        player.setPhone(newPhone);
        _playerMapper->update(player);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidAccessToken"),
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
        auto player = _playerMapper->findByPrimaryKey(userId);
        _checkEmailCode(player.getValueOfEmail(), code);

        _playerMapper->deleteOne(player);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidAccessToken"),
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

void PlayerManager::deactivatePhone(
        const int64_t userId,
        const string &code
) {
    try {
        auto player = _playerMapper->findByPrimaryKey(userId);
        _checkPhoneCode(player.getValueOfPhone(), code);

        _playerMapper->deleteOne(player);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found:" << e.what();
        throw ResponseException(
                i18n("invalidAccessToken"),
                ResultCode::NotAcceptable,
                k401Unauthorized
        );
    }
}

Json::Value PlayerManager::getUserInfo(
        const string &accessToken,
        int64_t userId
) {
    int64_t targetId = userId;
    NO_EXCEPTION(
            targetId = _userRedis->getIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper->findOne(orm::Criteria(
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

void PlayerManager::updateUserInfo(
        const int64_t userId,
        RequestJson request
) {
    auto player = _playerMapper->findByPrimaryKey(userId);
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
    _playerMapper->update(player);
}

string PlayerManager::getAvatar(const string &accessToken, int64_t userId) {
    int64_t targetId = userId;
    NO_EXCEPTION(
            targetId = _userRedis->getIdByAccessToken(accessToken);
    )
    try {
        auto player = _playerMapper->findOne(orm::Criteria(
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

bool PlayerManager::ipLimit(const string &ip) const {
    return _userRedis->tokenBucket(
            "ip:" + ip,
            _ipInterval,
            _ipMaxCount
    );
}

bool PlayerManager::emailLimit(const string &email) const {
    return _userRedis->tokenBucket(
            "email:" + email,
            _emailInterval,
            _emailMaxCount
    );
}

bool PlayerManager::phoneLimit(const string &phone) const {
    return _userRedis->tokenBucket(
            "phone:" + phone,
            _phoneInterval,
            _phoneMaxCount
    );
}

void PlayerManager::_checkEmailCode(const string &email, const string &code) {
    try {
        if (!_userRedis->checkEmailCode(email, code)) {
            throw ResponseException(
                    i18n("invalidCode"),
                    ResultCode::NotAcceptable,
                    k403Forbidden
            );
        }
        _userRedis->deleteEmailCode(email);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found: " << e.what();
        throw ResponseException(
                i18n("invalidEmail"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void PlayerManager::_checkPhoneCode(const string &phone, const string &code) {
    try {
        if (!_userRedis->checkPhoneCode(phone, code)) {
            throw ResponseException(
                    i18n("invalidCode"),
                    ResultCode::NotAcceptable,
                    k403Forbidden
            );
        }
        _userRedis->deletePhoneCode(phone);
    } catch (const redis_exception::KeyNotFound &e) {
        LOG_DEBUG << "Key not found: " << e.what();
        throw ResponseException(
                i18n("invalidPhone"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}
