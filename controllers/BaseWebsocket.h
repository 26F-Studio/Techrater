//
// Created by Parti on 2021/2/23.
//

#pragma once

#include <helpers/I18nHelper.h>
#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>
#include <magic_enum.hpp>
#include <structures/ExceptionHandlers.h>
#include <structures/HandlerManagerBase.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::ws::v1 {
    template<class controllerImpl, class handlerManagerImpl>
    class BaseWebsocket :
            public drogon::WebSocketController<controllerImpl>,
            public helpers::I18nHelper<controllerImpl> {
    public:
        BaseWebsocket() : _handlerManager(drogon::app().getPlugin<handlerManagerImpl>()) {};

        ~BaseWebsocket() override = default;

        void handleNewMessage(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                std::string &&message,
                const drogon::WebSocketMessageType &type
        ) final {
            using namespace drogon;
            using namespace techmino::helpers;
            using namespace techmino::structures;
            using namespace techmino::types;

            if (!connectionFilter(wsConnPtr)) {
                MessageJson(ErrorNumber::Error)
                        .setMessage(I18nHelper<controllerImpl>::i18n("invalidConnection"))
                        .to(wsConnPtr);
                return;
            }

            switch (type) {
                case WebSocketMessageType::Text:
                case WebSocketMessageType::Binary:
                    try {
                        auto request = BasicJson(message);
                        LOG_TRACE << request.stringify("  ");
                        requestHandler(wsConnPtr, request.ref());
                    } catch (const Json::RuntimeError &) {
                        MessageJson(ErrorNumber::Failed)
                                .setMessage(I18nHelper<controllerImpl>::i18n("invalidMessage"))
                                .to(wsConnPtr);
                    }
                    break;
                case WebSocketMessageType::Ping:
                    wsConnPtr->send(message, WebSocketMessageType::Pong);
                    break;
                case WebSocketMessageType::Pong:
                    break;
                case WebSocketMessageType::Close:
                    wsConnPtr->forceClose();
                    break;
                case WebSocketMessageType::Unknown:
                    LOG_WARN << "Message from " << wsConnPtr->peerAddr().toIpPort() << " is Unknown";
                    break;
            }
        }

    protected:
        virtual bool connectionFilter(const drogon::WebSocketConnectionPtr &wsConnPtr) {
            return wsConnPtr->connected();
        }

        virtual void requestHandler(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request
        ) {
            using namespace techmino::helpers;
            using namespace techmino::types;

            if (!request["action"].isInt()) {
                MessageJson(ErrorNumber::Failed)
                        .setMessage(I18nHelper<controllerImpl>::i18n("invalidAction"))
                        .to(wsConnPtr);
            }
            RequestJson requestJson(request["data"]);
            return _handlerManager->process(
                    request["action"].asInt(),
                    wsConnPtr,
                    requestJson
            );
        }

    private:
        structures::HandlerManagerBase<handlerManagerImpl> *_handlerManager;
    };
}
