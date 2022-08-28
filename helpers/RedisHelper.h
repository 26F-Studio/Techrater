//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/drogon.h>
#include <cpp_redis/cpp_redis>

namespace techmino::helpers {
    class RedisHelper : public trantor::NonCopyable {
    public:
        explicit RedisHelper(std::string BaseKey = CMAKE_PROJECT_NAME);

        void connect(
                const std::string &host = "127.0.0.1",
                size_t port = 6379,
                int db = 0,
                uint32_t timeout = 0,
                int32_t retries = 0,
                uint32_t interval = 0
        );

        void disconnect();

        [[nodiscard]] bool tokenBucket(
                const std::string &key,
                const std::chrono::microseconds &restoreInterval,
                const uint64_t &maxCount
        );

        virtual ~RedisHelper() = default;

    protected:
        void del(const std::string &key);

        std::vector<bool> exists(const std::vector<std::string> &keys);

        void expire(const std::string &key, const std::chrono::seconds &ttl);

        void expire(const std::vector<std::tuple<std::string, std::chrono::seconds>> &params);

        std::string get(const std::string &key);

        void setAdd(const std::string &key, const std::vector<std::string> &values);

        void setAdd(const std::vector<std::pair<std::string, std::vector<std::string>>> &params);

        int64_t setCard(const std::string &key);

        std::vector<std::string> setGetMembers(const std::string &key);

        std::vector<std::vector<std::string>> setGetMembers(const std::vector<std::string> &keys);

        bool setIsMember(const std::string &key, const std::string &value);

        void setRemove(const std::string &key, const std::vector<std::string> &values);

        void setRemove(const std::vector<std::pair<std::string, std::vector<std::string>>> &params);

        void set(const std::string &key, const std::string &value);

        void setEx(
                const std::string &key,
                int ttl,
                const std::string &value
        );

        void setEx(const std::vector<std::tuple<std::string, int, std::string>> &params);

    private:
        cpp_redis::client _redisClient;
        std::string _baseKey;
    };
}



