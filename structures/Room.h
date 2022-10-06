//
// Created by particleg on 2021/10/8.
//

#pragma once

#include <drogon/drogon.h>
#include <helpers/DataJson.h>
#include <helpers/I18nHelper.h>
#include <helpers/MessageJson.h>
#include <helpers/RequestJson.h>
#include <plugins/ConnectionManager.h>

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

        Room(Room &&room) noexcept;

        ~Room() override;

        bool empty(bool all = false) const;

        bool full() const;

        [[nodiscard]] bool checkPassword(const std::string &password) const;

        void updatePassword(const std::string &password);

        void subscribe(int64_t userId);

        void unsubscribe(int64_t userId);

        [[nodiscard]] uint64_t countPlaying() const;

        [[nodiscard]] uint64_t countSpectator() const;

        [[nodiscard]] uint64_t countStandby() const;

        [[nodiscard]] Json::Value parse(bool details = false) const;

        void publish(const helpers::MessageJson &message, int64_t excludedId = -1);

        Json::Value getData() const;

        Json::Value updateData(const Json::Value &data);

        Json::Value getInfo() const;

        Json::Value updateInfo(const Json::Value &data);

        void appendChat(Json::Value &&chat);

        void matchStart(bool force = false);

        bool cancelStart();

        void matchEnd(bool force = false);


    private:
        [[nodiscard]] uint64_t countGamer() const;

    public:
        const std::string roomId{drogon::utils::getUuid()};
        std::atomic<State> state{State::Standby};
        std::atomic<uint64_t> capacity, startTimerId;

    private:
        mutable std::shared_mutex _dataMutex, _playerMutex, _chatMutex;
        plugins::ConnectionManager *_connectionManager;
        std::string _passwordHash;
        helpers::DataJson _info, _data;
        std::unordered_set<int64_t> _playerSet;
        std::vector<Json::Value> _chatList;
    };
}
