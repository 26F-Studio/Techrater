//
// Created by Parti on 2021/3/27.
//

#include <controllers/Websocket.h>
#include <magic_enum.hpp>
#include <structures/Player.h>
#include <types/Action.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::ws::v2;
using namespace techmino::types;

Websocket::Websocket() :
        _connectionManager(app().getPlugin<ConnectionManager>()),
        _roomManager(app().getPlugin<RoomManager>()) {}

void Websocket::handleNewConnection(
        const HttpRequestPtr &req,
        const WebSocketConnectionPtr &wsConnPtr
) {
    auto id = req->getAttributes()->get<int64_t>("id");
    wsConnPtr->setContext(make_shared<Player>(id));
    wsConnPtr->setPingMessage("", chrono::seconds(10));
    _connectionManager->subscribe(wsConnPtr);
}

void Websocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {
    const auto &player = wsConnPtr->getContext<Player>();
    if (player) {
        if (!player->getRoomId().empty()) {
            try {
                _roomManager->roomLeave(enum_integer(Action::RoomLeave), wsConnPtr);
            } catch (const internal::BaseException &e) {
                LOG_DEBUG << "Closed without leaving room: " << e.what();
            }
        }
        _connectionManager->unsubscribe(wsConnPtr);
    }
}

std::string Websocket::reason(const string &param) const {
    return i18n(param);
}
