//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/ConnectionManager.h>
#include <strategies/GlobalOnlineCount.h>
#include <types/Action.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

GlobalOnlineCount::GlobalOnlineCount() : MessageHandlerBase(enum_integer(Action::GlobalOnlineCount)) {}

void GlobalOnlineCount::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    handleExceptions([&]() {
        MessageJson(_action).setData(
                app().getPlugin<ConnectionManager>()->count()
        ).to(wsConnPtr);
    }, _action, wsConnPtr);
}
