//
// Created by ParticleG on 2022/2/9.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::types;

MessageJson::MessageJson(MessageType messageType) :
        BasicJson() { setMessageType(messageType); }

MessageJson::MessageJson(Json::Value json, MessageType messageType) :
        BasicJson(std::move(json)) { setMessageType(messageType); }

MessageJson::MessageJson(const string &raw, MessageType messageType) :
        BasicJson(raw) { setMessageType(messageType); }

MessageJson::MessageJson(int action, MessageType messageType) :
        MessageJson(messageType) { setAction(action); }

MessageJson &MessageJson::setMessageType(MessageType type) {
    _value["errno"] = enum_integer(type);
    return *this;
}

MessageJson &MessageJson::setAction(int action) {
    _value["action"] = action;
    return *this;
}

MessageJson &MessageJson::setData(Json::Value data) {
    if (!data.empty()) {
        _value["data"] = std::move(data);
    } else {
        _value.removeMember("data");
    }
    return *this;
}

MessageJson &MessageJson::setReason(const string &reason) {
    if (!reason.empty()) {
        _value["reason"] = reason;
    } else {
        _value.removeMember("reason");
    }
    return *this;
}

MessageJson &MessageJson::setMessage(const string &message) {
    if (!message.empty()) {
        _value["message"] = message;
    } else {
        _value.removeMember("message");
    }
    return *this;
}

void MessageJson::to(const WebSocketConnectionPtr &connectionPtr) const {
    if (connectionPtr->connected()) {
        if (enum_cast<MessageType>(_value["errno"].asInt()).value_or(MessageType::Success) == MessageType::Error) {
            connectionPtr->shutdown(CloseCode::kViolation, stringify());
        } else {
            connectionPtr->send(stringify());
        }
    }
}
