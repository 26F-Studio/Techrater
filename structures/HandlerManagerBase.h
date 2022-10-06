//
// Created by particleg on 2021/10/10.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <structures/HandlerFactory.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::structures {
    template<class T>
    class HandlerManagerBase : public drogon::Plugin<T>,
                               public helpers::I18nHelper<T> {
    public:
        void process(
                int action,
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                helpers::RequestJson &request
        ) {
            using namespace helpers;
            using namespace types;

            try {
                const auto &handler = _handlerFactory.getHandler(action);
                if (const auto message = handler.filter(wsConnPtr, request)) {
                    MessageJson(action, MessageType::Failed)
                            .setMessage(message.value())
                            .to(wsConnPtr);
                    return;
                }
                handler.process(wsConnPtr, request);
            } catch (const action_exception::ActionNotFound &) {
                MessageJson(action, MessageType::Failed)
                        .setMessage(I18nHelper<T>::i18n("invalidArguments"))
                        .to(wsConnPtr);
            }
        }

        virtual ~HandlerManagerBase() = default;

    protected:
        HandlerFactory<MessageHandlerBase> _handlerFactory;
    };
}
