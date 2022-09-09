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

MessageJson::MessageJson() : BasicJson() { setMessageType(MessageType::Client); }

MessageJson::MessageJson(Json::Value json) : BasicJson(std::move(json)) {}

MessageJson::MessageJson(const string &raw) : BasicJson(raw) {}

MessageJson::MessageJson(int action) : MessageJson() { setAction(action); }

MessageJson &MessageJson::setMessageType(MessageType type) {
    _value["type"] = string(enum_name(type));
    return *this;
}

MessageJson &MessageJson::setAction(int action) {
    _value["action"] = action;
    return *this;
}

MessageJson &MessageJson::setData(Json::Value data) {
    _value["data"] = std::move(data);
    return *this;
}

MessageJson &MessageJson::setData() {
    _value.removeMember("data");
    return *this;
}

MessageJson &MessageJson::setReason(const exception &e) {
    setReason(e.what());
    return *this;
}

MessageJson &MessageJson::setReason(const drogon::orm::DrogonDbException &e) {
    setReason(e.base().what());
    return *this;
}

MessageJson &MessageJson::setReason(const string &reason) {
    _value["reason"] = reason;
    return *this;
}

MessageJson &MessageJson::sendTo(const WebSocketConnectionPtr &connectionPtr) {
    if (connectionPtr->connected()) {
        connectionPtr->send(stringify());
    }
    return *this;
}

MessageJson &MessageJson::closeWith(const WebSocketConnectionPtr &connectionPtr) {
    if (connectionPtr->connected()) {
        connectionPtr->shutdown(CloseCode::kViolation, stringify());
    }
    return *this;
}


