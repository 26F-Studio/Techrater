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
     * @param playerId: int64_t
     * @param _room: shared_ptr of the player joined room
     */
    class Player : public PlayerBase {
    public:
        explicit Player(int64_t playerId);

        Player(Player &&player) noexcept;

        ~Player() override;

        [[nodiscard]] std::shared_ptr<Room> getRoom() const;

        void setRoom(std::shared_ptr<Room> room);

        void reset();

    public:
        const drogon_model::techrater::Player playerInfo;

    private:
        mutable std::shared_mutex _sharedMutex;
        std::shared_ptr<Room> _room;
    };
}