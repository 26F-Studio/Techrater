//
// Created by particleg on 2021/9/29.
//

#include <structures/RedisToken.h>

using namespace std;
using namespace techmino::structures;

RedisToken::RedisToken(
        string refresh,
        string access
) : _refreshToken(std::move(refresh)),
    _accessToken(std::move(access)) {}

RedisToken::RedisToken(
        RedisToken &&redisToken
) noexcept: _refreshToken(std::move(redisToken._refreshToken)),
            _accessToken(std::move(redisToken._accessToken)) {}

string &RedisToken::access() { return _accessToken; }

Json::Value RedisToken::parse() const {
    Json::Value result;
    result["refreshToken"] = _refreshToken;
    result["accessToken"] = _accessToken;
    return result;
}

