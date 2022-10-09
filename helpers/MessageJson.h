//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/WebSocketConnection.h>
#include <helpers/BasicJson.h>
#include <types/ErrorNumber.h>

namespace techmino::helpers {
    class MessageJson : public BasicJson {
    public:
        explicit MessageJson(types::ErrorNumber messageType = types::ErrorNumber::Success);

        explicit MessageJson(const std::string &raw, types::ErrorNumber messageType = types::ErrorNumber::Success);

        explicit MessageJson(int action, types::ErrorNumber messageType = types::ErrorNumber::Success);

        MessageJson &setMessageType(types::ErrorNumber type);

        MessageJson &setAction(int action);

        MessageJson &setData(Json::Value data = {});

        MessageJson &setMessage(const std::string &message);

        MessageJson &setReason(const std::string &reason);

        void to(const drogon::WebSocketConnectionPtr &connectionPtr) const;
    };
}
