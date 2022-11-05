//
// Created by ParticleG on 2022/2/6.
//

#include <utils/data.h>
#include <random>

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
