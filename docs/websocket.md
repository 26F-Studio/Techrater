# Techrater WebSocket API

## Hosts

- `cafuuchino1.3322.org:10026`
- `cafuuchino2.3322.org:10026`
- `cafuuchino3.3322.org:10026`
- `cafuuchino4.3322.org:10026`
- `cafuuchino5.3322.org:10026`

## Base Path

`/techmino/ws/v1`

## Failed Message Structure

### Message

```json
{
  "errno": 1,
  // Action number (Optional if message is not related to an action)
  "action": 1001,
  // Failed message's I18N path
  "message": "WebSocket.handleNewMessage.invalidConnection",
  // Internal reasons (Optional)
  "reason": "No player object"
}
```

## Error Closure Structure

### Reason

```json
{
  "errno": -1,
  // Action number (Optional if message is not related to an action)
  "action": 1001,
  // Failed message's I18N path
  "message": "WebSocket.handleNewMessage.invalidConnection",
  // Internal reasons (Optional)
  "reason": "No player object"
}
```

## Error Number List

```c++
enum class ErrorNumber {
    // Error message, would close the connection
    Error = -1,
    // Success message
    Success = 0,
    // Failure message
    Failed
};
```

## Action Number List

```c++
enum class Action {
    /// Game actions
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
```

## Get/Update Array

### Get Array

```json
[
  {
    "path": "rules.endCondition.time",
    "value": 600
  },
  {
    "path": "path.to.configs.2.test",
    "value": {
      "foo": "a",
      "bar": "b"
    }
  }
]
```

### Update Array

```json
[
  "rules.endCondition.time",
  "path.to.configs.2.test"
]
```

## Target Types

- User: Any valid WebSocket connection's establisher
- Player: The one who is actually playing in the room
- Spectator: The one who is spectating the game
- Admin: The one who has management privileges in the room

## Role List

```c++
enum class Role {
    Normal,
    Admin,
    Super,
};
```

## Game Actions

### 1100 Game End

#### Description

Indicate that game has ended.

#### Request

None

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0
}
```

### 1101 Game Ready

#### Description

Indicate that all *players* are ready.

#### Request

None

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0
}
```

### 1102 Game Start

#### Description

Indicate that game has started.

#### Request

None

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": 123456789
}
```

## Player Actions

### 1200 Player Config

#### Description

Notify other players that local player config has changed.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // Base64 encoded config object
  "data": "BASE64_STRING"
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "config": "BASE64_STRING"
  }
}
```

### 1201 Player Finish

#### Description

Notify other players that local player has finished the game.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // Data of the current game
  "data": ANY_OBJECT
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "data": ANY_OBJECT
  }
}
```

### 1202 Player Group

#### Description

Notify other players that local player has changed the group.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": GROUP_NUMBER
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "group": GROUP_NUMBER
  }
}
```

### 1203 Player Ready

#### Description

Notify other players that local player has changed the ready state.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // True for ready, false for cancel
  "data": true
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "isReady": true
  }
}
```

### 1204 Player Role

#### Description

Request changing other player's role.

If current player's role is equal to the requesting role, current player's role would swap with the target player.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    "playerId": 111,
    "role": "Admin"
  }
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 111,
    "role": "Admin"
  }
}

// Below would also be sent if swap happens
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "role": "Normal"
  }
}
```

### 1205 Player State

#### Description

Notify other players that local player has changed the custom playing state.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // Custom state string
  "data": "ANY_STRING"
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "customState": "ANY_STRING"
  }
}
```

### 1206 Player Stream

#### Description

Stream game playing data.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // Base64 string data
  "data": "BASE64_STRING"
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "data": "BASE64_STRING"
  }
}
```

### 1207 Player Type

#### Description

Change current player's type in the room.

If the room is full, the player can't change from `Spectator` to `Gamer`.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // 'Spectator' or 'Gamer'
  "data": "Gamer"
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "type": "Gamer"
  }
}
```

## Room Actions

### 1300 Room Chat

#### Description

Send chat to the room. If player is global admin and `roomId` is specified, the message would send to the specified
room.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "message": "ANY_STRING"
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 123,
    "message": "ANY_STRING"
  }
}
```

### 1301 Room Create

#### Description

Create a room.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "roomId": "HEX_STRING",
    "capacity": 2,
    "state": "Playing",
    "count": {
      "Gamer": 2,
      "Spectator": 2
    },
    "private": false,
    "info": ANY_OBJECT,
    "data": ANY_OBJECT,
    "players": [],
    "chats": []
  }
}
```

#### Response

##### Targets

The ***user*** who created the room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  // Room ID
  "data": "HEX_STRING"
}
```

### 1302 Room Data Get

#### Description

Access current room's data. Can access other room's data by specifying `roomId` if player is global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING"
  }
}
```

#### Response

##### Targets

The ***user*** who send the request

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  // Room data
  "data": ANY_OBJECT
}
```

### 1303 Room Data Update

#### Description

Update current room's data, need to be the room admin. Can access other room's data by specifying `roomId` if player is
global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "data": UPDATE_ARRAY
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 11,
    "data": ANY_OBJECT
  }
}
```

### 1304 Room Info Get

#### Description

Access current room's data. Can access other room's data by specifying `roomId`.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional.
    "roomId": "HEX_STRING"
  }
}
```

#### Response

##### Targets

The ***user*** who send the request

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  // Room info
  "data": ANY_OBJECT
}
```

### 1305 Room Info Update

#### Description

Update current room's data, need to be the room admin. Can access other room's data by specifying `roomId` if player is
global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "info": UPDATE_ARRAY
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 11,
    "info": ANY_OBJECT
  }
}
```

### 1306 Room Join

#### Description

Join a room.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    "roomId": "HEX_STRING",
    // Optional.
    "password": "ANY_STRING"
  }
}
```

#### Response

##### Targets

- The ***user*** who joined the room
- All ***users*** in the specific room

##### Message

```json
// To the user who joined the room
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "roomId": "HEX_STRING",
    "capacity": 2,
    "state": "Playing",
    "count": {
      "Gamer": 2,
      "Spectator": 2
    },
    "private": false,
    "info": ANY_OBJECT,
    "data": ANY_OBJECT,
    "players": [
      {
        "playerId": 11,
        "group": 1,
        "role": "Admin",
        "type": "Gamer",
        "state": "Playing",
        "config": "BASE64_STRING",
        "history": "BASE64_STRING_CONTACTED"
      },
      {
        "playerId": 12,
        "group": 2,
        "role": "Normal",
        "type": "Gamer",
        "state": "Frozen",
        "config": "BASE64_STRING",
        "history": "BASE64_STRING_CONTACTED"
      },
      {
        "playerId": 13,
        "group": 0,
        "role": "Normal",
        "type": "Spectator",
        "state": "Standby",
        "config": "BASE64_STRING",
        // Empty if spectating
        "history": ""
      },
      {
        "playerId": 14,
        "group": 0,
        "role": "Normal",
        "type": "Gamer",
        "state": "Standby",
        "config": "BASE64_STRING",
        // Empty if spectating
        "history": ""
      }
    ],
    "chats": [
      {
        "playerId": 11,
        "message": "ANY_STRING"
      },
      {
        "playerId": 12,
        "message": "ANY_STRING"
      },
      {
        "playerId": 11,
        "message": "ANY_STRING"
      },
      {
        "playerId": 13,
        "message": "ANY_STRING"
      }
    ]
  }
}

// To other users in the room
{
  "action": ACTION_NUMBER,
  "errno": 0,
  // Joined player's info
  "data": {
    "playerId": 15,
    "group": 0,
    "role": "Normal",
    "type": "Spectator",
    "state": "Standby",
    "config": "BASE64_STRING",
    "history": ""
  }
}
```

### 1307 Room Kick

#### Description

Kick a ***user*** in current room, need to be the room admin. Can kick other room's player by specifying `roomId` if
player is global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional.
    "roomId": "HEX_STRING",
    "playerId": 13
  }
}
```

#### Response

##### Targets

- The ***user*** who is kicked
- All ***users*** in the specific room

##### Message

```json
// To the user who was kicked
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "executorId": 11,
    "playerId": 13
  }
}

// To other users in the room
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "executorId": 11,
    "playerId": 13
  }
}
```

### 1308 Room Leave

#### Description

Leave current room.

#### Request

```json
{
  "action": ACTION_NUMBER
}
```

#### Response

##### Targets

- The ***user*** who leaves the room
- All ***users*** in the specific room

##### Message

```json
// To the user who leaves the room
{
  "action": ACTION_NUMBER,
  "errno": 0
}

// To other users in the room
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 11
  }
}
```

### 1309 Room List

#### Description

List rooms according to `pageIndex` and `pageSize`.

#### Request

```json
{
  "action": ACTION_NUMBER,
  // Optional
  "data": {
    "pageIndex": 2,
    "pageSize": 10
  }
}
```

#### Response

##### Targets

The ***user*** who send the request

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": [
    {
      "roomId": "HEX_STRING",
      "capacity": 2,
      "state": "Playing",
      "count": {
        "Gamer": 2,
        "Spectator": 2
      },
      "info": ANY_OBJECT
    },
    {
      "roomId": "HEX_STRING",
      "capacity": 4,
      "state": "Standby",
      "count": {
        "Gamer": 3,
        "Spectator": 1
      },
      "info": ANY_OBJECT
    },
    {
      "roomId": "HEX_STRING",
      "capacity": 5,
      "state": "Ready",
      "count": {
        "Gamer": 5,
        "Spectator": 10
      },
      "info": ANY_OBJECT
    }
  ]
}
```

### 1310 Room Password

#### Description

Update current room's password, need to be the room admin. Can update other room's password by specifying `roomId` if
player is global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "password": "ANY_STRING"
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": ACTION_NUMBER,
  "errno": 0,
  "data": {
    "playerId": 11,
    "password": "ANY_STRING"
  }
}
```

### 1311 Room Remove

#### Description

Remove current room, need to be the room admin. Can remove other room by specifying `roomId` if player is global admin.

#### Request

```json
{
  "action": ACTION_NUMBER,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING"
  }
}
```

#### Response

##### Targets

- Executor
- All ***users*** in the specific room

##### Message

- Executor

```json
{
  "action": ACTION_NUMBER,
  "errno": 0
}
```

- All ***users*** in the specific room

```json
{
  "action": ACTION_NUMBER,
  "errno": 0
}
```

