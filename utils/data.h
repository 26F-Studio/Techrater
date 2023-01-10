//
// Created by ParticleG on 2022/2/3.
//

#pragma once

#include <drogon/drogon.h>

namespace techmino::internal {
    template<typename T>
    concept JsonTypes =
    std::convertible_to<T, bool> ||
    std::convertible_to<T, Json::Int> ||
    std::convertible_to<T, Json::Int64> ||
    std::convertible_to<T, Json::UInt> ||
    std::convertible_to<T, Json::UInt64> ||
    std::convertible_to<T, double> ||
    std::convertible_to<T, Json::String>;
};

namespace techmino::utils::data {
    std::string randomString(uint64_t length);

    uint64_t randomUniform(uint64_t start = 0, uint64_t size = 99999999);

    std::string join(std::vector<std::string> &&tokens);

    std::string join(std::vector<std::string> &&tokens, char delimiter);

    std::string join(std::vector<std::string> &&tokens, std::string &&delimiter);

    void quote(std::string &str, const std::string &quoter = R"(")");

    std::string quote(std::string &&str, const std::string &quoter = R"(")");

    void ltrim(std::string &s);

    std::string ltrim(std::string &&s);

    void rtrim(std::string &s);

    std::string rtrim(std::string &&s);

    void trim(std::string &s);

    std::string trim(std::string &&s);
}



