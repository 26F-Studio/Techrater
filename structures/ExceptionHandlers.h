//
// Created by ParticleG on 2022/2/8.
//

#pragma once

#include <drogon/drogon.h>
#include <magic_enum.hpp>
#include <helpers/I18nHelper.h>
#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>
#include <helpers/ResponseJson.h>
#include <structures/Exceptions.h>

namespace techmino::structures {

#define NO_EXCEPTION(expressions) try {expressions} catch (...) {}

    template<class T>
    class RequestJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                drogon::FilterCallback &failedCb
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace techmino::helpers;
            using namespace techmino::structures;
            using namespace techmino::types;

            try {
                mainFunction();
            } catch (const json_exception::InvalidFormat &e) {
                ResponseJson(k400BadRequest, ResultCode::InvalidFormat)
                        .setMessage(I18nHelper<T>::i18n("invalidFormat"))
                        .setReason(e)
                        .to(failedCb);
            } catch (const json_exception::WrongType &e) {
                ResponseJson(k400BadRequest, ResultCode::InvalidArguments)
                        .setMessage(I18nHelper<T>::i18n("invalidArguments"))
                        .setReason(e)
                        .to(failedCb);
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                ResponseJson(k500InternalServerError, ResultCode::InternalError)
                        .setMessage(I18nHelper<T>::i18n("internalError"))
                        .to(failedCb);
            }
        }

        ~RequestJsonHandler() override = default;
    };

    template<class T>
    class ResponseJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                helpers::ResponseJson &response
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace techmino::helpers;
            using namespace techmino::structures;
            using namespace techmino::types;

            try {
                mainFunction();
            } catch (const ResponseException &e) {
                response = e.toJson();
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                response.setStatusCode(k500InternalServerError)
                        .setResultCode(ResultCode::DatabaseError)
                        .setMessage(I18nHelper<T>::i18n("databaseError"));
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                response.setStatusCode(k500InternalServerError)
                        .setResultCode(ResultCode::InternalError)
                        .setMessage(I18nHelper<T>::i18n("internalError"));
            }
        }

        virtual ~ResponseJsonHandler() = default;
    };

    template<class T>
    class MessageJsonHandler : public helpers::I18nHelper<T> {
    public:
        void handleExceptions(
                const std::function<void()> &mainFunction,
                int action,
                const drogon::WebSocketConnectionPtr &wsConnPtr
        ) const {
            using namespace drogon;
            using namespace std;
            using namespace techmino::helpers;
            using namespace techmino::structures;
            using namespace techmino::types;

            try {
                mainFunction();
            } catch (const MessageException &e) {
                e.toJson().setAction(action).to(wsConnPtr);
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                MessageJson(action, ErrorNumber::Error)
                        .setMessage(I18nHelper<T>::i18n("databaseError"))
                        .to(wsConnPtr);
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                MessageJson(action, ErrorNumber::Error)
                        .setMessage(I18nHelper<T>::i18n("internalError"))
                        .to(wsConnPtr);
            }
        }

        virtual ~MessageJsonHandler() = default;
    };

    template<class T>
    class HttpRequestHandler : public helpers::I18nHelper<T> {
    public:
        HttpRequestHandler() = default;

        [[maybe_unused]] HttpRequestHandler(const std::string &ip, uint16_t port) :
                httpClient(drogon::HttpClient::newHttpClient(ip, port)),
                httpsClient(drogon::HttpClient::newHttpClient(ip, port, true)) {}

        [[nodiscard]] helpers::ResponseJson request(
                drogon::HttpMethod method,
                const std::string &path,
                const std::vector<std::pair<std::string, types::JsonValue>> &constraints = {},
                const std::vector<std::pair<std::string, std::string>> &headers = {},
                const Json::Value &body = Json::nullValue,
                bool useSsl = true
        ) const {
            using namespace drogon;
            using namespace helpers;
            using namespace magic_enum;
            using namespace std;
            using namespace structures;
            using namespace types;

            auto req = body.isNull() ? HttpRequest::newHttpRequest() : HttpRequest::newHttpJsonRequest(body);
            req->setPath(path);
            req->setMethod(method);
            for (const auto &header: headers) {
                req->addHeader(header.first, header.second);
            }
            auto [result, responsePtr] = useSsl ? httpsClient->sendRequest(req, 10) : httpClient->sendRequest(req, 10);
            if (result != ReqResult::Ok) {
                throw ResponseException(
                        I18nHelper<T>::i18n("networkError"),
                        ResultCode::NetworkError,
                        drogon::k503ServiceUnavailable
                );
            }
            RequestJson response{responsePtr};
            if (responsePtr->statusCode() != k200OK) {
                throw ResponseException(
                        response["message"].asString(),
                        enum_cast<ResultCode>(response["code"].asUInt()).value_or(ResultCode::Unknown),
                        responsePtr->statusCode()
                );
            }
            try {
                for (const auto &constraint: constraints) {
                    response.require(constraint.first, constraint.second);
                }
            } catch (const exception &e) {
                throw ResponseException(
                        I18nHelper<T>::i18n("networkError"),
                        ResultCode::NetworkError,
                        drogon::k503ServiceUnavailable
                );
            }
            return {ResponseJson{responsePtr}};
        }

        ~HttpRequestHandler() override = default;

    protected:
        drogon::HttpClientPtr httpClient, httpsClient;

        void setClient(const std::string &ip, uint16_t port) {
            httpClient = drogon::HttpClient::newHttpClient(ip, port);
            httpsClient = drogon::HttpClient::newHttpClient(ip, port, true);
        }
    };
}