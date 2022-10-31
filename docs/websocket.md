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
    /// Global actions
    GlobalOnlineCount = 1000,
    
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

## 10xx Global Actions

### 1000 Global Online Count

#### Description

Get online player count.

#### Request

```json
{
  "action": 1000
}
```

#### Response

##### Targets

The ***user*** who send the request

##### Message

```json
{
  "action": 1000,
  "errno": 0,
  "data": 114
}
```

## 11xx Match Actions

### 1100 Match End

#### Description

Indicate that match has ended.

#### Request

None

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": 1100,
  "errno": 0
}
```

### 1101 Match Ready

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
  "action": 1101,
  "errno": 0
}
```

### 1102 Match Start

#### Description

Indicate that match has started.

#### Request

None

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": 1102,
  "errno": 0,
  "data": {
    "seed": 123456789
  }
}
```

## 12xx Player Actions

### 1200 Player Config

#### Description

Update current player's config.
If in a room, notify other players that local player config has changed.

#### Request

```json
{
  "action": 1200,
  // Base64 encoded config object
  "data": "BASE64_STRING"
}
```

#### Response

##### Targets

- The ***user*** who sent the request
- If in a room, notify all ***users*** in the room

##### Message

- To the user who sent the request

```json
{
  "action": 1200,
  "errno": 0,
  "data": "BASE64_STRING"
}
```

- To all users in the room

```json
{
  "action": 1200,
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
  "action": 1201,
  // Data of the current game
  "data": {
    "foo": "bar"
  }
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": 1201,
  "errno": 0,
  "data": {
    "playerId": 123,
    "data": {
      "foo": "bar"
    }
  }
}
```

### 1202 Player Group

#### Description

Notify other players that local player has changed the group.

#### Request

```json
{
  "action": 1202,
  // Group number
  "data": 2
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": 1202,
  "errno": 0,
  "data": {
    "playerId": 123,
    "group": 2
  }
}
```

### 1203 Player Ready

#### Description

Notify other players that local player has changed the ready state.

#### Request

```json
{
  "action": 1203,
  // True for ready, false for standby
  "data": true
}
```

#### Response

##### Targets

All ***users*** in the room

##### Message

```json
{
  "action": 1203,
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
  "action": 1204,
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
  "action": 1204,
  "errno": 0,
  "data": {
    "playerId": 111,
    "role": "Admin"
  }
}
```

> Below would also be sent if swap happens

```json
{
  "action": 1204,
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
  "action": 1205,
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
  "action": 1205,
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
  "action": 1206,
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
  "action": 1206,
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
  "action": 1207,
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
  "action": 1207,
  "errno": 0,
  "data": {
    "playerId": 123,
    "type": "Gamer"
  }
}
```

## 13xx Room Actions

### 1300 Room Chat

#### Description

Send chat to the room. If player is global admin and `roomId` is specified, the message would send to the specified
room.

#### Request

```json
{
  "action": 1300,
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
  "action": 1300,
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
  "action": 1301,
  "errno": 0,
  "data": {
    "capacity": 2,
    "info": {
      "foo": "bar"
    },
    "data": {
      "foo": "bar"
    },
    // Optional.
    "password": "114514"
  }
}
```

#### Response

##### Targets

The ***user*** who created the room

##### Message

```json
{
  "action": 1301,
  "errno": 0,
  "data": {
    "roomId": "HEX_STRING",
    "capacity": 2,
    "seed": 123456789,
    "state": "Standby",
    "count": {
      "Gamer": 1,
      "Spectator": 0
    },
    "private": true,
    "info": {
      "foo": "bar"
    },
    "data": {
      "foo": "bar"
    },
    "players": [
      {
        "playerId": 11,
        "group": 1,
        "role": "Admin",
        "type": "Gamer",
        "state": "Playing",
        "config": "BASE64_STRING",
        "history": ""
      }
    ],
    "chats": []
  }
}
```

### 1302 Room Data Get

#### Description

Access current room's data. Can access other room's data by specifying `roomId` if player is global admin.

#### Request

```json
{
  "action": 1302,
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
  "action": 1302,
  "errno": 0,
  // Room data
  "data": {
    "foo": "bar"
  }
}
```

### 1303 Room Data Update

#### Description

Update current room's data, need to be the room admin. Can access other room's data by specifying `roomId` if player is
global admin.

#### Request

```json
{
  "action": 1303,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "data": [
      {
        "path": "foo.bar",
        "data": "demo"
      }
    ]
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": 1303,
  "errno": 0,
  "data": {
    "playerId": 11,
    "data": {
      "foo": {
        "bar": "demo"
      }
    }
  }
}
```

### 1304 Room Info Get

#### Description

Access current room's data. Can access other room's data by specifying `roomId`.

#### Request

```json
{
  "action": 1304,
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
  "action": 1304,
  "errno": 0,
  // Room info
  "data": {
    "foo": "bar"
  }
}
```

### 1305 Room Info Update

#### Description

Update current room's data, need to be the room admin. Can access other room's data by specifying `roomId` if player is
global admin.

#### Request

```json
{
  "action": 1305,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING",
    "info": [
      {
        "path": "foo.0.bar",
        "data": {
          "demo": true
        }
      }
    ]
  }
}
```

#### Response

##### Targets

All ***users*** in the specific room

##### Message

```json
{
  "action": 1305,
  "errno": 0,
  "data": {
    "playerId": 11,
    "info": {
      "foo": [
        {
          "bar": {
            "demo": true
          }
        }
      ]
    }
  }
}
```

### 1306 Room Join

#### Description

Join a room.

#### Request

```json
{
  "action": 1306,
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

- To the user who joined the room

```json
{
  "action": 1306,
  "errno": 0,
  "data": {
    "roomId": "HEX_STRING",
    "capacity": 2,
    "seed": 123456789,
    "state": "Playing",
    "count": {
      "Gamer": 2,
      "Spectator": 2
    },
    "private": false,
    "info": {
      "foo": "bar"
    },
    "data": {
      "foo": "bar"
    },
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
```

- To other users in the room

```json
{
  "action": 1306,
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
  "action": 1307,
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

- To the user who's been kicked

```json
{
  "action": 1307,
  "errno": 0,
  "data": {
    "executorId": 11,
    "playerId": 13
  }
}
```

- To other users in the room

```json
{
  "action": 1307,
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
  "action": 1308
}
```

#### Response

##### Targets

- The ***user*** who leaves the room
- All ***users*** in the specific room

##### Message

- To the user who leaves the room

```json
{
  "action": 1308,
  "errno": 0
}
```

- To other users in the room

```json
{
  "action": 1308,
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
  "action": 1309,
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
  "action": 1309,
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
      "info": {
        "foo": "bar"
      }
    },
    {
      "roomId": "HEX_STRING",
      "capacity": 4,
      "state": "Standby",
      "count": {
        "Gamer": 3,
        "Spectator": 1
      },
      "info": {
        "foo": "bar"
      }
    },
    {
      "roomId": "HEX_STRING",
      "capacity": 5,
      "state": "Ready",
      "count": {
        "Gamer": 5,
        "Spectator": 10
      },
      "info": {
        "foo": "bar"
      }
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
  "action": 1310,
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
  "action": 1310,
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
  "action": 1311,
  "data": {
    // Optional. Need permission to set roomId
    "roomId": "HEX_STRING"
  }
}
```

#### Response

##### Targets

- The request executor
- All ***users*** in the specific room

##### Message

- To the executor

```json
{
  "action": 1311,
  "errno": 0
}
```

- To all ***users*** in the specific room

```json
{
  "action": 1311,
  "errno": 0
}
```