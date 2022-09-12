//
// Created by Particle_G on 2021/3/04.
//

#include <helpers/MessageJson.h>
#include <magic_enum.hpp>
#include <plugins/RoomManager.h>
#include <strategies/RoomList.h>
#include <types/Action.h>
#include <types/JsonValue.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::strategies;
using namespace techmino::structures;
using namespace techmino::types;

RoomList::RoomList() : MessageHandlerBase(enum_integer(Action::RoomList)) {}

void RoomList::process(const WebSocketConnectionPtr &wsConnPtr, RequestJson &request) const {
    // TODO: Implement search logics
    handleExceptions([&]() {
        uint64_t pageIndex = 0, pageSize = 10;

        if (request.check("pageIndex", JsonValue::UInt64)) {
            pageIndex = request["pageIndex"].asUInt64();
        }

        if (request.check("pageSize", JsonValue::UInt64)) {
            pageSize = request["pageSize"].asUInt64();
        }

        MessageJson(_action).setData(
                app().getPlugin<RoomManager>()->listRoom(pageIndex, pageSize)
        ).sendTo(wsConnPtr);

    }, _action, wsConnPtr);
}
