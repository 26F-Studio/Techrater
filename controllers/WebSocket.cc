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
using namespace techmino::types;
using namespace techmino::ws::v1;

WebSocket::WebSocket() : _connectionManager(app().getPlugin<ConnectionManager>()) {}

void WebSocket::handleNewConnection(
        const HttpRequestPtr &req,
        const WebSocketConnectionPtr &wsConnPtr
) {
    try {
        const auto playerId = req->getAttributes()->get<int64_t>("playerId");
        wsConnPtr->setContext(make_shared<Player>(playerId));
        wsConnPtr->setPingMessage("", chrono::seconds(5));
        _connectionManager->subscribe(wsConnPtr);
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        MessageJson(ErrorNumber::Error)
                .setMessage(i18n("playerNotFound"))
                .setReason(e.base().what())
                .to(wsConnPtr);
    } catch (const exception &e) {
        LOG_ERROR << e.what();
        MessageJson(ErrorNumber::Error)
                .setMessage(i18n("connectionFailed"))
                .setReason(e.what())
                .to(wsConnPtr);
    }
}

void WebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) {
    wsConnPtr->forceClose();
}

bool WebSocket::connectionFilter(const WebSocketConnectionPtr &wsConnPtr) {
    return wsConnPtr->connected() && wsConnPtr->getContext<Player>();
}
