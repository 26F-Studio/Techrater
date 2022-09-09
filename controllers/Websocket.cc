//
// Created by Parti on 2021/3/27.
//

#include <controllers/Websocket.h>
#include <magic_enum.hpp>
#include <structures/Player.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::ws::v1;

Websocket::Websocket() : _connectionManager(app().getPlugin<ConnectionManager>()) {}

void Websocket::handleNewConnection(
        const HttpRequestPtr &req,
        const WebSocketConnectionPtr &wsConnPtr
) {
    auto id = req->getAttributes()->get<int64_t>("id");
    wsConnPtr->setContext(make_shared<Player>(id));
    wsConnPtr->setPingMessage("", chrono::seconds(5));
    _connectionManager->subscribe(wsConnPtr);
}

void Websocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {
    _connectionManager->unsubscribe(wsConnPtr);
}

bool Websocket::connectionFilter(const WebSocketConnectionPtr &wsConnPtr) {
    return wsConnPtr->connected() && wsConnPtr->getContext<Player>();
}
