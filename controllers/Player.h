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
                    Player::getData,
                    "/data",
                    drogon::Get,
                    "techmino::filters::GetAccessToken",
                    "techmino::filters::CheckPlayerId"
            );
            METHOD_ADD(
                    Player::updateData,
                    "/data",
                    drogon::Put,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::PlayerUpdateData"
            );
        METHOD_LIST_END

        void getData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void updateData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        techmino::plugins::PlayerManager *_playerManager;
    };
}
