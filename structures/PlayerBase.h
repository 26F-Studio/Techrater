//
// Created by g29624 on 2022/8/30.
//

#pragma once

#include <drogon/drogon.h>

namespace techmino::structures {
    class PlayerBase {
    public:
        explicit PlayerBase(int64_t userId);

        virtual ~PlayerBase() = default;

        const int64_t userId;
    };
}