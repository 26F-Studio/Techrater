//
// Created by Parti on 2021/4/9.
//

#pragma once

namespace techmino::types {
    enum class Action {
        /// Forward actions
        ForwardData = 1000,

        /// Game actions
        GameEnd = 1100,
        GameReady,
        GameSpectate,
        GameStart,

        /// Player actions
        PlayerConfig = 1200,
        PlayerFinish,
        PlayerGroup,
        PlayerPing,
        PlayerReady,
        PlayerRole,
        PlayerState,
        PlayerType,

        /// Room actions
        RoomCreate = 1300,
        RoomDataGet,
        RoomDataUpdate,
        RoomInfoGet,
        RoomInfoUpdate,
        RoomJoin,
        RoomKick,
        RoomLeave,
        RoomList,
        RoomPassword,
        RoomRemove,
    };
}
