//
// Created by Particle_G on 2021/3/04.
//

#pragma once

#include <structures/ExceptionHandlers.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::strategies {
    class PlayerGroup :
            public structures::MessageHandlerBase,
            public structures::MessageJsonHandler<PlayerGroup> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        PlayerGroup();

        bool filter(const drogon::WebSocketConnectionPtr &wsConnPtr, helpers::RequestJson &request) override;

        void process(const drogon::WebSocketConnectionPtr &wsConnPtr, helpers::RequestJson &request) override;
    };
}
