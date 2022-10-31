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
#include <structures/PlayerBase.h>

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
        class Member : public PlayerBase {
        public:
            enum class Role {
                Normal,
                Admin,
                Super,
            };
            enum class State {
                Standby,
                Ready,
                Playing,
                Finished,
            };
            enum class Type {
                Gamer,
                Spectator,
            };

        public:
            explicit Member(
                    int64_t playerId,
                    Role role = Role::Normal,
                    State state = State::Standby,
                    Type type = Type::Spectator
            );

            Member(Member &&member) noexcept;

            void setConfig(std::string &&config);

            bool hasConfig() const;

            void setCustomState(std::string &&customState);

            void appendHistory(const std::string &history);

            [[nodiscard]] std::string history() const;

            [[nodiscard]] Json::Value info() const;

            void reset();

        public:
            std::atomic<uint64_t> group{0};
            std::atomic<Role> role;
            std::atomic<State> state;
            std::atomic<Type> type;

        private:
            mutable std::shared_mutex _dataMutex, _historyMutex;
            std::string _config, _customState, _history;
        };

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

        void subscribe(Member &&member);

        void unsubscribe(int64_t userId);

        [[nodiscard]] Json::Value parse(bool details = false) const;

        void publish(const helpers::MessageJson &message, int64_t excludedId = -1);

        Json::Value getData() const;

        Json::Value updateData(const Json::Value &data);

        Json::Value getInfo() const;

        Json::Value updateInfo(const Json::Value &data);

        void appendChat(Json::Value &&chat);

        void matchTryStart(bool force = false);

        bool matchCancelStart();

        void matchTryEnd(bool force = false);

    private:
        [[nodiscard]] uint64_t countGamer() const;

        [[nodiscard]] uint64_t countSpectator() const;

        [[nodiscard]] uint64_t countPlaying() const;

        [[nodiscard]] bool isAllReady() const;

    public:
        const std::string roomId{drogon::utils::getUuid()};
        std::atomic<State> state{State::Standby};
        std::atomic<uint64_t> capacity, startTimerId;

    private:
        mutable std::shared_mutex _dataMutex, _playerMutex, _chatMutex;
        plugins::ConnectionManager *_connectionManager;
        std::string _passwordHash;
        helpers::DataJson _info, _data;
        std::unordered_map<int64_t, Member> _memberMap;
        std::vector<Json::Value> _chatList;
    };
}
