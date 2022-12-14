//
// Created by Particle_G on 2021/2/17.
//

#pragma once

#include <drogon/WebSocketController.h>
#include <helpers/RequestJson.h>

namespace techmino::structures {
    class MessageHandlerBase {
    public:
        explicit MessageHandlerBase(int action) : _action(action) {}

        virtual std::optional<std::string> filter(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                helpers::RequestJson &request
        ) const { return std::nullopt; }

        virtual void process(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                helpers::RequestJson &request
        ) const = 0;

        virtual ~MessageHandlerBase() = default;

    protected:
        const int _action;
    };
}