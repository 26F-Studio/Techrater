//
// Created by particleg on 2021/10/2.
//

#pragma once

#include <drogon/drogon.h>
#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <types/JsonValue.h>
#include <types/ErrorNumber.h>
#include <types/ResultCode.h>

namespace techmino::internal {
    class BaseException : public std::exception {
    public:
        explicit BaseException(std::string message);

        ~BaseException() noexcept override = default;

        [[nodiscard]] const char *what() const noexcept override;

    protected:
        const std::string _message;
    };
}

namespace techmino::structures {
    class ResponseException : public internal::BaseException {
    public:
        explicit ResponseException(
                std::string message,
                types::ResultCode resultCode = types::ResultCode::InternalError,
                drogon::HttpStatusCode statusCode = drogon::HttpStatusCode::k500InternalServerError
        );

        explicit ResponseException(
                std::string message,
                const std::exception &e,
                types::ResultCode resultCode = types::ResultCode::InternalError,
                drogon::HttpStatusCode statusCode = drogon::HttpStatusCode::k500InternalServerError
        );

        [[nodiscard]] helpers::ResponseJson toJson() const noexcept;

    private:
        const std::string _reason;
        const types::ResultCode _resultCode;
        const drogon::HttpStatusCode _statusCode;
    };

    class MessageException : public internal::BaseException {
    public:
        explicit MessageException(
                std::string message,
                bool error = false
        );

        explicit MessageException(
                std::string message,
                const std::exception &e,
                bool error = false
        );

        [[nodiscard]] helpers::MessageJson toJson() const noexcept;

    private:
        const std::string _reason;
        const bool _error;
    };

    class EmailException : public internal::BaseException {
    public:
        explicit EmailException(std::string message);
    };

    namespace json_exception {
        class InvalidFormat : public internal::BaseException {
        public:
            explicit InvalidFormat(std::string message);
        };

        class WrongType : public internal::BaseException {
        public:
            explicit WrongType(types::JsonValue valueType);
        };
    }

    namespace redis_exception {
        class KeyNotFound : public internal::BaseException {
        public:
            explicit KeyNotFound(std::string message);
        };

        class FieldNotFound : public internal::BaseException {
        public:
            [[maybe_unused]] explicit FieldNotFound(std::string message);
        };
    }

    namespace action_exception {
        class ActionNotFound : public internal::BaseException {
        public:
            explicit ActionNotFound(std::string message);
        };
    }
}
