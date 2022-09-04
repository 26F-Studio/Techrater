//
// Created by Parti on 2021/2/23.
//

#pragma once

#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>
#include <magic_enum.hpp>
#include <structures/ExceptionHandlers.h>
#include <structures/HandlerManagerBase.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::ws::v1 {
    template<class controllerImpl, class handlerManagerImpl>
    class BaseWebsocket : public drogon::WebSocketController<controllerImpl> {
    public:
        BaseWebsocket() : _handlerManager(drogon::app().getPlugin<handlerManagerImpl>()) {};

        void handleNewMessage(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                std::string &&message,
                const drogon::WebSocketMessageType &type
        ) final {
            using namespace drogon;
            using namespace techmino::helpers;
            using namespace techmino::structures;

            if (!wsConnPtr->connected()) {
                return;
            }

            switch (type) {
                case WebSocketMessageType::Text:
                case WebSocketMessageType::Binary:
                    requestHandler(wsConnPtr, BasicJson(message).ref());
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
                    LOG_WARN << "Message from " << wsConnPtr->peerAddr().toIpPort()
                             << " is Unknown";
                    break;
            }
        }

        ~BaseWebsocket() override = default;

    protected:
        virtual void requestHandler(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                const Json::Value &request
        ) {
            using namespace techmino::helpers;
            using namespace techmino::types;

            if (!request["action"].isInt()) {
                MessageJson message;
                message.setMessageType(MessageType::Failed);
                message.setReason(reason("invalidAction"));
                message.sendTo(wsConnPtr);
            }
            RequestJson requestJson(request["data"]);
            return _handlerManager->process(
                    request["action"].asInt(),
                    wsConnPtr,
                    requestJson
            );
        }

        [[nodiscard]] virtual std::string reason(const std::string &param) const = 0;

    private:
        structures::HandlerManagerBase<handlerManagerImpl> *_handlerManager;
    };
}
