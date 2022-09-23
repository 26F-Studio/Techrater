//
// Created by Parti on 2021/3/27.
//

#include <controllers/WebSocket.h>
#include <magic_enum.hpp>
#include <structures/Player.h>

using namespace drogon;
using namespace magic_enum;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::ws::v1;

WebSocket::WebSocket() : _connectionManager(app().getPlugin<ConnectionManager>()) {}

void WebSocket::handleNewConnection(
        const HttpRequestPtr &req,
        const WebSocketConnectionPtr &wsConnPtr
) {
    auto id = req->getAttributes()->get<int64_t>("id");
    wsConnPtr->setContext(make_shared<Player>(id));
    wsConnPtr->setPingMessage("", chrono::seconds(5));
    _connectionManager->subscribe(wsConnPtr);
}

void WebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {
    NO_EXCEPTION(_connectionManager->unsubscribe(wsConnPtr);)
}

bool WebSocket::connectionFilter(const WebSocketConnectionPtr &wsConnPtr) {
    return wsConnPtr->connected() && wsConnPtr->getContext<Player>();
}
