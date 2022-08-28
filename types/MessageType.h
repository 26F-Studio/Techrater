//
// Created by ParticleG on 2022/3/11.
//

#pragma once

#include <drogon/drogon.h>

namespace techmino::types {
    enum class MessageType {
        Error,
        Failed,
        Client,
        Server,
    };
}