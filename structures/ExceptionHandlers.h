//
// Created by ParticleG on 2022/2/8.
//

#pragma once

#include <drogon/drogon.h>
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
                ResponseJson response;
                response.setStatusCode(k400BadRequest);
                response.setResultCode(ResultCode::InvalidFormat);
                // message.setMessage(this->i18n(e.what()));
                response.setMessage(I18nHelper<T>::i18n("invalidFormat"));
                response.setReason(e);
                response.httpCallback(failedCb);
            } catch (const json_exception::WrongType &e) {
                ResponseJson response;
                response.setStatusCode(k400BadRequest);
                response.setResultCode(ResultCode::InvalidArguments);
                response.setMessage(I18nHelper<T>::i18n("invalidArguments"));
                response.httpCallback(failedCb);
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                ResponseJson response;
                response.setStatusCode(k500InternalServerError);
                response.setResultCode(ResultCode::InternalError);
                response.setMessage(I18nHelper<T>::i18n("internalError"));
                response.setReason(e);
                response.httpCallback(failedCb);
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
                response.setStatusCode(e.statusCode());
                response(e.toJson());
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                response.setStatusCode(k500InternalServerError);
                response.setResultCode(ResultCode::DatabaseError);
                response.setMessage(I18nHelper<T>::i18n("databaseError"));
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                response.setStatusCode(k500InternalServerError);
                response.setResultCode(ResultCode::InternalError);
                response.setMessage(I18nHelper<T>::i18n("internalError"));
                response.setReason(e);
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
                MessageJson message(action);
                message.setReason(I18nHelper<T>::i18n(e.what()));
                if (e.error) {
                    message.setMessageType(MessageType::Error);
                    wsConnPtr->shutdown(CloseCode::kViolation, message.stringify());
                } else {
                    message.setMessageType(MessageType::Failed);
                    wsConnPtr->send(message.stringify());
                }
            } catch (const orm::DrogonDbException &e) {
                LOG_ERROR << e.base().what();
                MessageJson message(action);
                message.setReason(I18nHelper<T>::i18n("databaseError"));
                message.setMessageType(MessageType::Error);
                wsConnPtr->shutdown(CloseCode::kViolation, message.stringify());
            } catch (const exception &e) {
                LOG_ERROR << e.what();
                MessageJson message(action);
                message.setReason(I18nHelper<T>::i18n("internalError"));
                message.setMessageType(MessageType::Error);
                wsConnPtr->shutdown(CloseCode::kViolation, message.stringify());
            }
        }

        virtual ~MessageJsonHandler() = default;
    };
}