//
// Created by particleg on 2021/10/8.
//

#pragma once

#include <drogon/drogon.h>
#include <helpers/DataJson.h>
#include <helpers/I18nHelper.h>
#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>

namespace techmino::structures {
    /**
     * @brief Gaming room to store players and game data
     *
     * @param roomId: string
     * @param state: State
     * @param endCondition: EndCondition
     * @param leftLimit: uint64_t
     * @param capacity: uint64_t
     * @param startTimerId: uint64_t
     * @param endTimerId: uint64_t
     * @param transferNode: InetAddress
     * @param _passwordHash: string
     * @param _info: json
     * @param _data: json
     * @param _playerSet: unordered_set(id)
     */
    class Room : helpers::I18nHelper<Room> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;
        using WebSocketConnectionRef = std::weak_ptr<drogon::WebSocketConnection>;

    public:
        enum class State {
            Playing,
            Ready,
            Standby,
        };

        explicit Room(
                uint64_t capacity,
                const std::string &password,
                Json::Value info,
                Json::Value data
        );

        ~Room() override;

        bool empty(bool all = false);

        bool full();

        [[nodiscard]] bool checkPassword(const std::string &password) const;

        void updatePassword(const std::string &password);

        void subscribe(const drogon::WebSocketConnectionPtr &wsConnPtr);

        void unsubscribe(int64_t playerId);

        int64_t getFirstPlayerId();

        Json::Value parse(bool details = false);

        void publish(const helpers::MessageJson &message, int64_t excludedId = -1);

        Json::Value getData() const;

        Json::Value updateData(const Json::Value &data);

        Json::Value getInfo() const;

        Json::Value updateInfo(const Json::Value &data);

        void appendChat(Json::Value &&chat);

        void matchTryStart(bool force = false);

        bool cancelStart();

        void matchTryEnd(bool force = false);

    private:
        uint64_t countGamer();

        uint64_t countRemaining();

        uint64_t countSpectator();

        bool isAllReady();

    public:
        const std::string roomId{drogon::utils::getUuid()};
        std::atomic<State> state{State::Standby};
        std::atomic<uint64_t> capacity, seed;

    private:
        mutable std::shared_mutex _dataMutex, _playerMutex, _chatMutex;
        std::string _passwordHash;
        helpers::DataJson _info, _data;
        std::unordered_map<int64_t, WebSocketConnectionRef> _playerMap;
        std::vector<Json::Value> _chatList;
        std::atomic<bool> _needClean{false};
        std::atomic<uint64_t> _cleanTimerId, _startTimerId;

        void clean();
    };
}
