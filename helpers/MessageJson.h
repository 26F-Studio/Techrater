//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <drogon/WebSocketConnection.h>
#include <helpers/BasicJson.h>
#include <types/MessageType.h>

namespace techmino::helpers {
    class MessageJson : public BasicJson {
    public:
        explicit MessageJson(types::MessageType messageType = types::MessageType::Success);

        explicit MessageJson(Json::Value json, types::MessageType messageType = types::MessageType::Success);

        explicit MessageJson(const std::string &raw, types::MessageType messageType = types::MessageType::Success);

        explicit MessageJson(int action, types::MessageType messageType = types::MessageType::Success);

        MessageJson &setMessageType(types::MessageType type);

        MessageJson &setAction(int action);

        MessageJson &setData(Json::Value data = {});

        MessageJson &setMessage(const std::string &message);

        MessageJson &setReason(const std::string &reason);

        void to(const drogon::WebSocketConnectionPtr &connectionPtr) const;
    };
}
