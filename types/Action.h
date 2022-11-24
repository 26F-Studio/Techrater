//
// Created by Parti on 2021/4/9.
//

#pragma once

namespace techmino::types {
    enum class Action {
        /// Global actions
        GlobalOnlineCount = 1000,
        GlobalNotification,

        /// Match actions
        MatchEnd = 1100,
        MatchReady,
        MatchStart,

        /// Player actions
        PlayerConfig = 1200,
        PlayerFinish,
        PlayerGroup,
        PlayerReady,
        PlayerRole,
        PlayerState,
        PlayerStream,
        PlayerType,

        /// Room actions
        RoomChat = 1300,
        RoomCreate,
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
