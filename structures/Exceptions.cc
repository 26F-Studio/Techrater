//
// Created by particleg on 2021/10/2.
//

#include <helpers/ResponseJson.h>
#include <magic_enum.hpp>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::internal;
using namespace techmino::structures;
using namespace techmino::types;

BaseException::BaseException(string message) : _message(std::move(message)) {}

char const *BaseException::what() const noexcept { return _message.c_str(); }

ResponseException::ResponseException(
        string message,
        ResultCode resultCode,
        HttpStatusCode statusCode
) : BaseException(std::move(message)), _resultCode(resultCode), _statusCode(statusCode) {}

ResponseException::ResponseException(
        string message,
        const exception &e,
        ResultCode resultCode,
        HttpStatusCode statusCode
) : BaseException(std::move(message)), _reason(e.what()), _resultCode(resultCode), _statusCode(statusCode) {}

ResponseJson ResponseException::toJson() const noexcept {
    return ResponseJson(_statusCode, _resultCode)
            .setMessage(_message)
            .setReason(_reason);
}

MessageException::MessageException(
        string message,
        bool error
) : BaseException(std::move(message)), _error(error) {}

MessageException::MessageException(
        std::string message,
        const exception &e,
        bool error
) : BaseException(std::move(message)), _reason(e.what()), _error(error) {}

MessageJson MessageException::toJson() const noexcept {
    return MessageJson(_error ? MessageType::Error : MessageType::Failed)
            .setMessage(_message)
            .setReason(_reason);
}

EmailException::EmailException(
        string message
) : BaseException(std::move(message)) {}

json_exception::InvalidFormat::InvalidFormat(std::string message) :
        BaseException(std::move(message)) {}

json_exception::WrongType::WrongType(JsonValue valueType) :
        BaseException(string(enum_name(valueType))) {}

redis_exception::KeyNotFound::KeyNotFound(string message) :
        BaseException(std::move(message)) {}

[[maybe_unused]] redis_exception::FieldNotFound::FieldNotFound(string message) :
        BaseException(std::move(message)) {}

action_exception::ActionNotFound::ActionNotFound(string message) :
        BaseException(std::move(message)) {}