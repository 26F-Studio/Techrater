//
// Created by particleg on 2021/10/10.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <structures/HandlerFactory.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::structures {
    template<class handlerManagerImpl>
    class HandlerManagerBase : public drogon::Plugin<handlerManagerImpl> {
    public:
        void process(
                int action,
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                helpers::RequestJson &request
        ) {
            using namespace helpers;
            using namespace types;

            const auto &handler = _handlerFactory.getHandler(action);
            if (const auto reason = handler.filter(wsConnPtr, request)) {
                MessageJson(action, MessageType::Failed)
                        .setMessage(reason.value())
                        .to(wsConnPtr);
            }
            handler.process(wsConnPtr, request);
        }

        virtual ~HandlerManagerBase() = default;

    protected:
        HandlerFactory<MessageHandlerBase> _handlerFactory;
    };
}
