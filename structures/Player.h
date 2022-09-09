//
// Created by g29624 on 2022/8/30.
//

#pragma once

#include <models/Player.h>
#include <structures/PlayerBase.h>
#include <structures/Room.h>
#include <shared_mutex>

namespace techmino::structures {
    /**
     * @brief Connection between client and server
     *
     * @param userId: int64_t
     * @param group: uint64_t (default: 0)
     * @param role: Role (default: normal)
     * @param type: Type (default: spectator)
     * @param state: State (default: standby)
     * @param _roomId: string
     * @param _customState: string
     * @param _config: string
     * @param _pingList: json
     */
    class Player : public PlayerBase {
    public:
        enum class Role {
            Normal = 1,
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
        explicit Player(
                int64_t userId,
                Role role = Role::Normal,
                State state = State::Standby,
                Type type = Type::Spectator
        );

        Player(Player &&player) noexcept;

        ~Player() override;

        [[nodiscard]] std::shared_ptr<Room> getRoom() const;

        void setRoom(std::shared_ptr<Room> room);

        [[nodiscard]] std::string getCustomState() const;

        void setCustomState(std::string &&customState);

        [[nodiscard]] std::string getConfig() const;

        void setConfig(std::string &&config);

        [[nodiscard]] Json::Value info() const;

        void reset();

    public:
        const drogon_model::techrater::Player playerInfo;
        std::atomic<uint64_t> group{0};
        std::atomic<Role> role;
        std::atomic<State> state;
        std::atomic<Type> type;

    private:
        mutable std::shared_mutex _sharedMutex;
        std::shared_ptr<Room> _room;
        std::string _customState, _config;
    };
}