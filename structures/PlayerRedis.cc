//
// Created by particleg on 2021/9/29.
//

#include <structures/ExceptionHandlers.h>
#include <structures/PlayerRedis.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace std;
using namespace cpp_redis;
using namespace techmino::structures;
using namespace techmino::utils;

// TODO: Make some Redis actions run in pipeline

PlayerRedis::PlayerRedis(Expiration expiration) : RedisHelper("player"), _expiration(expiration) {}

PlayerRedis::PlayerRedis(PlayerRedis &&redis) noexcept: RedisHelper("player"), _expiration(redis._expiration) {}

RedisToken PlayerRedis::refresh(const string &refreshToken) {
    const auto userId = get("auth:refresh-id:" + refreshToken);
    del("auth:refresh-id:" + refreshToken);
    return generateTokens(userId);
}

RedisToken PlayerRedis::generateTokens(const string &userId) {
    NO_EXCEPTION(del("auth:refresh-id:" + get("auth:id-refresh:" + userId));)
    return {
            _generateRefreshToken(userId),
            _generateAccessToken(userId)
    };
}

bool PlayerRedis::checkEmailCode(const string &email, const string &code) {
    return get("auth:code:email:" + email) == code;
}

void PlayerRedis::deleteEmailCode(const string &email) {
    del("auth:code:email:" + email);
}

void PlayerRedis::setEmailCode(const string &email, const string &code) {
    setEx(
            "auth:code:email:" + email,
            _expiration.getEmailSeconds(),
            code
    );
}

int64_t PlayerRedis::getIdByAccessToken(const string &accessToken) {
    return stoll(get("auth:access-id:" + accessToken));
}

string PlayerRedis::_generateRefreshToken(const string &userId) {
    auto refreshToken = crypto::keccak(drogon::utils::getUuid());
    setEx({{
                   "auth:id-refresh:" + userId,
                   _expiration.getRefreshSeconds(),
                   refreshToken
           },
           {
                   "auth:refresh-id:" + refreshToken,
                   _expiration.getRefreshSeconds(),
                   userId
           }});
    return refreshToken;
}

string PlayerRedis::_generateAccessToken(const string &userId) {
    auto accessToken = crypto::blake2B(drogon::utils::getUuid());
    setEx(
            "auth:access-id:" + accessToken,
            _expiration.getAccessSeconds(),
            userId
    );
    return accessToken;
}



