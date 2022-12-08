//
// Created by particleg on 12/6/22.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>

namespace techmino::plugins {
    class Configurator :
            public drogon::Plugin<Configurator>,
            public helpers::I18nHelper<Configurator> {
    public:
        static constexpr const char *projectName = CMAKE_PROJECT_NAME;
    public:
        Configurator() = default;

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;
    };
}
