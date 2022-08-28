//
// Created by particleg on 2021/9/29.
//

#include <structures/PlayerRedis.h>
#include <utils/crypto.h>

using namespace drogon;
using namespace std;
using namespace cpp_redis;
using namespace techmino::structures;
using namespace techmino::utils;

// TODO: Make some Redis actions run in pipeline

PlayerRedis::PlayerRedis(Expiration expiration) : RedisHelper("user"), _expiration(expiration) {}

PlayerRedis::PlayerRedis(PlayerRedis &&redis) noexcept: RedisHelper("user"), _expiration(redis._expiration) {}

RedisToken PlayerRedis::refresh(const string &refreshToken) {
    expire("auth:refresh:" + refreshToken, _expiration.refresh);
    return {
            refreshToken,
            _generateAccessToken(get("auth:refresh:" + refreshToken))
    };
}

RedisToken PlayerRedis::generateTokens(const string &userId) {
    return {
            _generateRefreshToken(userId),
            _generateAccessToken(userId)
    };
}

bool PlayerRedis::checkEmailCode(const string &email, const string &code) {
    return get("auth:code:email:" + email) == code;
}

bool PlayerRedis::checkPhoneCode(const string &phone, const string &code) {
    return get("auth:code:phone:" + phone) == code;
}

void PlayerRedis::deleteEmailCode(const string &email) {
    del("auth:code:email:" + email);
}

void PlayerRedis::deletePhoneCode(const string &phone) {
    del("auth:code:phone:" + phone);
}

void PlayerRedis::setEmailCode(const string &email, const string &code) {
    setEx(
            "auth:code:email:" + email,
            _expiration.getEmailSeconds(),
            code
    );
}

void PlayerRedis::setPhoneCode(const string &phone, const string &code) {
    setEx(
            "auth:code:phone:" + phone,
            _expiration.getPhoneSeconds(),
            code
    );
}

int64_t PlayerRedis::getIdByAccessToken(const string &accessToken) {
    return stoll(get("auth:access:" + accessToken));
}

string PlayerRedis::_generateRefreshToken(const string &userId) {
    auto refreshToken = crypto::keccak(drogon::utils::getUuid());
    setEx(
            "auth:refresh:" + refreshToken,
            _expiration.getRefreshSeconds(),
            userId
    );
    return refreshToken;
}

string PlayerRedis::_generateAccessToken(const string &userId) {
    auto accessToken = crypto::blake2B(drogon::utils::getUuid());
    setEx({{
                   "auth:id:" + userId,
                   _expiration.getAccessSeconds(),
                   accessToken
           },
           {
                   "auth:access:" + accessToken,
                   _expiration.getAccessSeconds(),
                   userId
           }});
    return accessToken;
}



