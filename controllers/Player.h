//
// Created by particleg on 2022/9/1.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/PlayerManager.h>
#include <structures/ExceptionHandlers.h>

namespace techmino::api::v1 {
    class Player :
            public drogon::HttpController<Player>,
            public structures::ResponseJsonHandler<Player> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        Player();

        METHOD_LIST_BEGIN
            METHOD_ADD(
                    Player::getInfo,
                    "/info",
                    drogon::Get,
                    "techmino::filters::GetAccessToken",
                    "techmino::filters::CheckPlayerId"
            );
            METHOD_ADD(
                    Player::updateInfo,
                    "/info",
                    drogon::Put,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::PlayerUpdateInfo"
            );
            METHOD_ADD(
                    Player::getAvatar,
                    "/avatar",
                    drogon::Get,
                    "techmino::filters::GetAccessToken",
                    "techmino::filters::CheckPlayerId"
            );
        METHOD_LIST_END

        void getInfo(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void updateInfo(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void getAvatar(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        techmino::plugins::PlayerManager *_playerManager;
    };
}
