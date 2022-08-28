//
// Created by ParticleG on 2022/2/9.
//

#include <structures/Exceptions.h>
#include <helpers/RedisHelper.h>
#include <utils/crypto.h>
#include <utils/datetime.h>

using namespace cpp_redis;
using namespace drogon;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::structures;
using namespace techmino::utils;

RedisHelper::RedisHelper(std::string BaseKey) : _baseKey(std::move(BaseKey)) {}

void RedisHelper::connect(
        const string &host,
        size_t port,
        int db,
        uint32_t timeout,
        int32_t retries,
        uint32_t interval
) {
    _redisClient.connect(
            host,
            port,
            [=, this](const string &host, size_t port, client::connect_state status) {
                if (status == client::connect_state::dropped) {
                    LOG_ERROR << "client disconnected from " << host << ":" << port;
                    connect(host, port, db, timeout, retries, interval);
                } else if (status == client::connect_state::ok) {
                    _redisClient.select(db);
                    _redisClient.sync_commit();
                }
            },
            timeout,
            retries,
            interval
    );
    LOG_INFO << "Redis connected.";
}

void RedisHelper::disconnect() {
    if (_redisClient.is_connected()) {
        _redisClient.disconnect();
        LOG_INFO << "Redis disconnected.";
    }
}

bool RedisHelper::tokenBucket(
        const string &key,
        const chrono::microseconds &restoreInterval,
        const uint64_t &maxCount
) {
    const auto countKey = _baseKey + ":tokenBucket:count:" + key;
    const auto updatedKey = _baseKey + ":tokenBucket:updated:" + key;
    const auto maxTtl = chrono::duration_cast<chrono::seconds>(restoreInterval * maxCount);

    uint64_t countValue;
    try {
        countValue = stoull(get(countKey));
    } catch (...) {
        _redisClient.set(countKey, to_string(maxCount - 1));
        countValue = maxCount;
    }

    bool hasToken = true;
    try {
        const auto lastUpdated = get(updatedKey);
        const auto nowMicroseconds = datetime::toDate().microSecondsSinceEpoch();
        const auto generatedCount =
                (nowMicroseconds -
                 datetime::toDate(lastUpdated).microSecondsSinceEpoch()
                ) / restoreInterval.count() - 1;

        if (generatedCount >= 1) {
            _redisClient.set(updatedKey, datetime::toString(nowMicroseconds));
            _redisClient.incrby(countKey, static_cast<int>(generatedCount) - 1);
            hasToken = true;
        } else if (countValue > 0) {
            _redisClient.decr(countKey);
            hasToken = true;
        } else {
            hasToken = false;
        }
    } catch (...) {
        _redisClient.set(updatedKey, datetime::toString());
        _redisClient.set(countKey, to_string(maxCount - 1));
    }

    // Use sync methods to make sure the operation is completed.
    expire({{countKey,   maxTtl},
            {updatedKey, maxTtl}});
    return hasToken;
}

void RedisHelper::del(const string &key) {
    _redisClient.del({_baseKey + ":" + key});
    _redisClient.sync_commit();
}

vector<bool> RedisHelper::exists(const vector<string> &keys) {
    vector<string> tempKeys;
    transform(
            keys.begin(),
            keys.end(),
            back_inserter(tempKeys),
            [this](const auto &key) {
                return _baseKey + ":" + key;
            }
    );
    auto future = _redisClient.exists(tempKeys);
    _redisClient.sync_commit();
    const auto reply = future.get();

    if (reply.is_null()) {
        throw redis_exception::KeyNotFound({});
    }
    const auto &array = reply.as_array();
    vector<bool> result;
    transform(array.begin(), array.end(), back_inserter(result), [](const auto &item) {
        return item.as_integer();
    });
    return result;
}

void RedisHelper::expire(const string &key, const chrono::seconds &ttl) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.expire(tempKey, static_cast<int>(ttl.count()));
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
}

void RedisHelper::expire(const vector<tuple<string, chrono::seconds>> &params) {
    vector<future<reply>> futures;
    futures.reserve(params.size());
    for (const auto &[key, ttl]: params) {
        futures.push_back(_redisClient.expire(_baseKey + ":" + key, static_cast<int>(ttl.count())));
    }
    _redisClient.sync_commit();
    for (auto index = 0; index < futures.size(); ++index) {
        const auto reply = futures[index].get();
        if (reply.is_null()) {
            const auto &[key, _] = params[index];
            throw redis_exception::KeyNotFound(key);
        }
    }
}

string RedisHelper::get(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.get(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_string();
}


void RedisHelper::setAdd(const string &key, const vector<string> &values) {
    _redisClient.sadd(_baseKey + ":" + key, values);
    _redisClient.sync_commit();
}

void RedisHelper::setAdd(const vector<pair<string, vector<string>>> &params) {
    for (const auto &[key, values]: params) {
        _redisClient.sadd(_baseKey + ":" + key, values);
    }
    _redisClient.sync_commit();
}

int64_t RedisHelper::setCard(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.scard(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_integer();
}

vector<string> RedisHelper::setGetMembers(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.smembers(tempKey);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    const auto &array = reply.as_array();
    vector<string> members;
    transform(array.begin(), array.end(), back_inserter(members), [](const auto &item) {
        return item.as_string();
    });
    return members;
}

vector<vector<string>> RedisHelper::setGetMembers(const vector<string> &keys) {
    vector<future<reply>> futures;
    futures.reserve(keys.size());
    for (const auto &key: keys) {
        futures.push_back(_redisClient.smembers(_baseKey + ":" + key));
    }
    _redisClient.sync_commit();
    vector<vector<string>> result;
    for (auto index = 0; index < futures.size(); ++index) {
        const auto reply = futures[index].get();
        if (reply.is_null()) {
            throw redis_exception::KeyNotFound(keys[index]);
        }
        const auto &array = reply.as_array();
        vector<string> members;
        transform(array.begin(), array.end(), back_inserter(members), [](const auto &item) {
            return item.as_string();
        });
        result.push_back(members);
    }
    return result;
}

bool RedisHelper::setIsMember(const string &key, const string &value) {
    const auto tempKey = _baseKey + ":" + key;
    auto future = _redisClient.sismember(tempKey, value);
    _redisClient.sync_commit();
    const auto reply = future.get();
    if (reply.is_null()) {
        throw redis_exception::KeyNotFound(tempKey);
    }
    return reply.as_integer();
}

void RedisHelper::setRemove(const string &key, const vector<string> &values) {
    _redisClient.srem(_baseKey + ":" + key, values);
    _redisClient.sync_commit();
}

void RedisHelper::setRemove(const vector<pair<string, vector<string>>> &params) {
    for (const auto &[key, values]: params) {
        _redisClient.srem(_baseKey + ":" + key, values);
    }
    _redisClient.sync_commit();
}

void RedisHelper::set(const string &key, const string &value) {
    _redisClient.set(_baseKey + ":" + key, value);
    _redisClient.sync_commit();
}

void RedisHelper::setEx(
        const string &key,
        int ttl,
        const string &value
) {
    _redisClient.setex(_baseKey + ":" + key, ttl, value);
    _redisClient.sync_commit();
}

void RedisHelper::setEx(const vector<tuple<string, int, string>> &params) {
    for (const auto &[key, ttl, value]: params) {
        _redisClient.setex(_baseKey + ":" + key, ttl, value);
    }
    _redisClient.sync_commit();
}
