//
// Created by ParticleG on 2022/2/9.
//

#include <helpers/RedisHelper.h>
#include <range/v3/all.hpp>
#include <structures/Exceptions.h>
#include <utils/crypto.h>
#include <utils/datetime.h>

using ranges::to;
namespace view = ranges::view;
namespace views = ranges::views;

using namespace drogon;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::structures;
using namespace techmino::utils;

RedisHelper::RedisHelper(string BaseKey) :
        _baseKey(std::move(BaseKey)),
        _redisClient(app().getRedisClient()) {}

RedisHelper::~RedisHelper() {
    _redisClient->closeAll();
    LOG_INFO << "Redis disconnected.";
}

bool RedisHelper::tokenBucket(
        const string &key,
        const chrono::microseconds &restoreInterval,
        const uint64_t &maxCount
) {
    const auto countKey = "tokenBucket:count:" + key;
    const auto updatedKey = "tokenBucket:updated:" + key;
    const auto maxTtl = chrono::duration_cast<chrono::milliseconds>(restoreInterval * maxCount);

    bool hasToken = true;
    if (exists(countKey) && exists(updatedKey)) {
        const auto countValue = stoll(get(countKey));
        const auto lastUpdated = datetime::toDate(get(updatedKey)).microSecondsSinceEpoch();
        const auto currentTime = datetime::toDate().microSecondsSinceEpoch();
        const auto generatedCount = (currentTime - lastUpdated) / restoreInterval.count() - 1;

        if (generatedCount >= 1) {
            set(updatedKey, datetime::toString(currentTime));
            incrBy(countKey, generatedCount - 1);
        } else if (countValue > 0) {
            decrBy(countKey);
        } else {
            hasToken = false;
        }
    } else {
        set({{updatedKey, datetime::toString()},
             {countKey,   to_string(maxCount - 1)}});
    }

    pExpire({{countKey,   maxTtl},
             {updatedKey, maxTtl}});
    return hasToken;
}

int64_t RedisHelper::del(const string &key) {
    return _redisClient->execCommandSync<int64_t>(
            intCb,
            R"(del %s)", (_baseKey + ":" + key).c_str()
    );
}

bool RedisHelper::exists(const string &key) {
    return _redisClient->execCommandSync<bool>(
            intCb,
            R"(exists %s)", (_baseKey + ":" + key).c_str()
    );
}

[[maybe_unused]] bool RedisHelper::expire(const string &key, const chrono::seconds &ttl) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<bool>(
            intCb,
            R"(expire %s %lld)", tempKey.c_str(), ttl.count()
    );
}

[[maybe_unused]] vector<bool> RedisHelper::expire(const KeyPairs <chrono::seconds> &params) {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, ttl]: params) {
        transaction->execCommandAsync(
                traceCb, errorCb,
                R"(expire %s %lld)", (_baseKey + ":" + key).c_str(), ttl.count()
        );
    }
    promise<vector<bool>> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_DEBUG << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                vector<bool> results;
                results.reserve(resultsArray.size());
                for (const auto &item: resultsArray) {
                    results.emplace_back(item.asInteger());
                }
                resultsPromise.set_value(std::move(results));
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({});
            }
    );
    return resultsFuture.get();
}

vector<bool> RedisHelper::pExpire(const RedisHelper::KeyPairs<chrono::milliseconds> &params) {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, ttl]: params) {
        transaction->execCommandAsync(
                traceCb, errorCb,
                R"(pExpire %s %lld)", (_baseKey + ":" + key).c_str(), ttl.count()
        );
    }
    promise<vector<bool>> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                vector<bool> results;
                results.reserve(resultsArray.size());
                for (const auto &item: resultsArray) {
                    results.emplace_back(item.asInteger());
                }
                resultsPromise.set_value(std::move(results));
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({});
            }
    );
    return resultsFuture.get();
}

string RedisHelper::get(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<string>(
            [=](const nosql::RedisResult &result) {
                if (result.isNil()) {
                    throw redis_exception::KeyNotFound(tempKey);
                }
                return result.asString();
            },
            R"(get %s)", tempKey.c_str()
    );
}

int64_t RedisHelper::incrBy(const string &key, const int64_t &value) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<int64_t>(
            intCb,
            R"(incrBy %s %lld)", tempKey.c_str(), value
    );
}

int64_t RedisHelper::decrBy(const string &key, const int64_t &value) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<int64_t>(
            intCb,
            R"(decrBy %s %lld)", tempKey.c_str(), value
    );
}

RedisHelper::SimpleResult RedisHelper::set(const string &key, const string &value) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<SimpleResult>(
            simpleCb,
            R"(set %s %s)", tempKey.c_str(), value.c_str()
    );
}

RedisHelper::SimpleResults RedisHelper::set(const vector<tuple<string, string>> &params) {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, value]: params) {
        const auto tempKey = _baseKey + ":" + key;
        transaction->execCommandAsync(
                traceCb, errorCb,
                R"(set %s %s)", tempKey.c_str(), value.c_str()
        );
    }
    promise<SimpleResults> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_TRACE << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                SimpleResults results;
                results.reserve(resultsArray.size());
                for (const auto &item: resultsArray) {
                    results.emplace_back(item.asString() == "OK", item.asString());
                }
                resultsPromise.set_value(std::move(results));
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({});
            }
    );
    return resultsFuture.get();
}

RedisHelper::SimpleResult RedisHelper::setPx(
        const string &key,
        const string &value,
        const chrono::milliseconds &ttl
) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<SimpleResult>(
            simpleCb,
            R"(set %s %s px %lld)", tempKey.c_str(), value.c_str(), ttl.count()
    );
}

RedisHelper::SimpleResults RedisHelper::setPx(const vector<tuple<string, string, chrono::milliseconds>> &params) {
    const auto transaction = _redisClient->newTransaction();
    for (const auto &[key, value, ttl]: params) {
        const auto tempKey = _baseKey + ":" + key;
        transaction->execCommandAsync(
                traceCb, errorCb,
                R"(set %s %s px %lld)", tempKey.c_str(), value.c_str(), ttl.count()
        );
    }
    promise<SimpleResults> resultsPromise;
    auto resultsFuture = resultsPromise.get_future();
    transaction->execute(
            [&](const nosql::RedisResult &result) {
                LOG_DEBUG << result.getStringForDisplayingWithIndent();
                const auto &resultsArray = result.asArray();
                SimpleResults results;
                results.reserve(resultsArray.size());
                for (const auto &item: resultsArray) {
                    results.emplace_back(item.asString() == "OK", item.asString());
                }
                resultsPromise.set_value(std::move(results));
            },
            [&](const exception &err) {
                LOG_ERROR << err.what();
                resultsPromise.set_value({});
            }
    );
    return resultsFuture.get();
}

[[maybe_unused]] chrono::seconds RedisHelper::ttl(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<chrono::seconds>(
            [=](const nosql::RedisResult &result) {
                return chrono::seconds{result.asInteger()};
            },
            R"(ttl %s)", tempKey.c_str()
    );
}

[[maybe_unused]] chrono::milliseconds RedisHelper::pTtl(const string &key) {
    const auto tempKey = _baseKey + ":" + key;
    return _redisClient->execCommandSync<chrono::milliseconds>(
            [=](const nosql::RedisResult &result) {
                return chrono::milliseconds{result.asInteger()};
            },
            R"(pTtl %s)", tempKey.c_str()
    );
}
