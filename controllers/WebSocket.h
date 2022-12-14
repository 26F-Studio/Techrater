//
// Created by Parti on 2021/2/5.
//

#pragma once

#include <controllers/BaseWebsocket.h>
#include <filters/CheckAccessToken.h>
#include <helpers/I18nHelper.h>
#include <plugins/ConnectionManager.h>
#include <plugins/HandlerManager.h>

namespace techmino::ws::v1 {
    class WebSocket : public BaseWebsocket<WebSocket, plugins::HandlerManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        WS_PATH_LIST_BEGIN
            WS_PATH_ADD("/techmino/ws/v1", "techmino::filters::CheckAccessToken");
        WS_PATH_LIST_END

        WebSocket();

        void handleNewConnection(
                const drogon::HttpRequestPtr &req,
                const drogon::WebSocketConnectionPtr &wsConnPtr
        ) override;

        void handleConnectionClosed(const drogon::WebSocketConnectionPtr &wsConnPtr) override;

    private:
        plugins::ConnectionManager *_connectionManager;

        bool connectionFilter(const drogon::WebSocketConnectionPtr &wsConnPtr) override;
    };
}