//
// Created by Particle_G on 2021/3/04.
//

#pragma once

#include <structures/ExceptionHandlers.h>
#include <structures/MessageHandlerBase.h>

namespace techmino::strategies {
    class GlobalOnlineCount :
            public structures::MessageHandlerBase,
            public structures::MessageJsonHandler<GlobalOnlineCount> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        GlobalOnlineCount();

        void process(
                const drogon::WebSocketConnectionPtr &wsConnPtr,
                helpers::RequestJson &request
        ) const override;
    };
}
