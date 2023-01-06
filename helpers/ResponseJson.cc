//
// Created by ParticleG on 2022/2/9.
//

#include <helpers/ResponseJson.h>
#include <magic_enum.hpp>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::structures;
using namespace techmino::types;

ResponseJson::ResponseJson(
        HttpStatusCode statusCode,
        ResultCode resultCode
) : BasicJson(), _statusCode(statusCode) { setResultCode(resultCode); }

ResponseJson::ResponseJson(const HttpResponsePtr &res) {
    auto object = res->getJsonObject();
    if (!object) {
        throw json_exception::InvalidFormat(res->getJsonError());
    }
    _value = std::move(*object);
}

ResponseJson &ResponseJson::setResultCode(ResultCode code) {
    setResultCode(enum_integer(code));
    return *this;
}

ResponseJson &ResponseJson::setResultCode(uint32_t code) {
    _value["code"] = code;
    return *this;
}

ResponseJson &ResponseJson::setStatusCode(drogon::HttpStatusCode code) {
    _statusCode = code;
    return *this;
}

ResponseJson &ResponseJson::setData(Json::Value data) {
    _value["data"] = std::move(data);
    return *this;
}

ResponseJson &ResponseJson::setMessage(const string &message) {
    _value["message"] = message;
    return *this;
}

ResponseJson &ResponseJson::setReason(const string &reason) {
    _value["reason"] = reason;
    return *this;
}

ResponseJson &ResponseJson::setReason(const exception &e) {
    setReason(e.what());
    return *this;
}

void ResponseJson::to(const ResponseJson::HttpCallback &callback) const {
    auto httpJsonResponse = HttpResponse::newHttpJsonResponse(_value);
    httpJsonResponse->setStatusCode(_statusCode);
    callback(httpJsonResponse);
}
