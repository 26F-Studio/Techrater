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
        MessageJson();

        explicit MessageJson(Json::Value json);

        explicit MessageJson(const std::string &raw);

        explicit MessageJson(int action);

        MessageJson &setMessageType(types::MessageType type = types::MessageType::Client);

        MessageJson &setAction(int action);

        MessageJson &setData(Json::Value data);

        [[maybe_unused]] MessageJson &setData();

        MessageJson &setReason(const std::exception &e);

        [[maybe_unused]] MessageJson &setReason(const drogon::orm::DrogonDbException &e);

        MessageJson &setReason(const std::string &reason);

        MessageJson &sendTo(const drogon::WebSocketConnectionPtr &connectionPtr);

        MessageJson &closeWith(const drogon::WebSocketConnectionPtr &connectionPtr);
    };
}



