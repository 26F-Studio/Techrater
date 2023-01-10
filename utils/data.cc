//
// Created by ParticleG on 2022/2/6.
//

#include <random>
#include <range/v3/all.hpp>
#include <utils/data.h>

using ranges::to;
namespace view = ranges::view;
namespace views = ranges::views;

using namespace std;
using namespace techmino::utils;
using namespace trantor;

static const char charset[] = "0123456789";

string data::randomString(uint64_t length) {
    static mt19937 engine(random_device{}());
    static uniform_int_distribution<uint64_t> uniDist(0, sizeof(charset) - 2);
    string result;
    result.resize(length);
    for (auto &c: result) {
        c = charset[uniDist(engine)];
    }
    return result;
}

uint64_t data::randomUniform(uint64_t start, uint64_t size) {
    mt19937 generator(static_cast<unsigned int>(Date::now().microSecondsSinceEpoch()));
    uniform_int_distribution<uint64_t> dis(start, size);
    return dis(generator);
}


string data::join(vector<std::string> &&tokens) {
    return tokens | view::join | to<string>();
}

string data::join(vector<std::string> &&tokens, char delimiter) {
    return tokens | view::join(delimiter) | to<string>();
}

string data::join(vector<std::string> &&tokens, string &&delimiter) {
    return tokens | view::join(std::move(delimiter)) | to<string>();
}

void data::quote(string &str, const string &quoter) {
    str.insert(0, quoter);
    str.append(quoter);
}

string data::quote(string &&str, const string &quoter) {
    quote(str, quoter);
    return std::move(str);
}

void data::ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}

string data::ltrim(string &&s) {
    ltrim(s);
    return std::move(s);
}

void data::rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

string data::rtrim(string &&s) {
    rtrim(s);
    return std::move(s);
}

void data::trim(string &s) {
    rtrim(s);
    ltrim(s);
}

string data::trim(string &&s) {
    trim(s);
    return std::move(s);
}
