//
// Created by g29624 on 2022/8/30.
//

#pragma once

#include <structures/PlayerBase.h>
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
        Player(
                int64_t userId,
                Role role,
                State state,
                Type type
        );

        ~Player() override = default;

    public:
        std::atomic<uint64_t> group{0};
        std::atomic<Role> role;
        std::atomic<State> state;
        std::atomic<Type> type;

    private:
        mutable std::shared_mutex _sharedMutex;
        // TODO: Use shared_ptr<Room> instead of roomId
        std::string _roomId, _customState, _config;
        Json::Value _pingList;
    };
}