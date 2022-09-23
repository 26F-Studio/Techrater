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

ResponseJson &ResponseJson::setResultCode(ResultCode code) { setResultCode(enum_integer(code)); }

ResponseJson &ResponseJson::setResultCode(uint32_t code) { _value["code"] = code; }

ResponseJson &ResponseJson::setStatusCode(drogon::HttpStatusCode code) { _statusCode = code; }

ResponseJson &ResponseJson::setMessage(const string &message) { _value["message"] = message; }

ResponseJson &ResponseJson::setData(Json::Value data) { _value["data"] = std::move(data); }

ResponseJson &ResponseJson::setReason(const exception &e) { setReason(e.what()); }

[[maybe_unused]] ResponseJson &ResponseJson::setReason(const drogon::orm::DrogonDbException &e) { setReason(e.base().what()); }

ResponseJson &ResponseJson::setReason(const string &reason) { _value["reason"] = reason; }

void ResponseJson::to(const ResponseJson::HttpCallback &callback) const {
    auto httpJsonResponse = HttpResponse::newHttpJsonResponse(_value);
    httpJsonResponse->setStatusCode(_statusCode);
    callback(httpJsonResponse);
}
