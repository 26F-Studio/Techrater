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
    "type": "Failed",
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
    "type": "Error",
    // Action number (Optional if message is not related to an action)
    "action": 1001,
    // Failed message's I18N path
    "message": "WebSocket.handleNewMessage.invalidConnection",
    // Internal reasons (Optional)
    "reason": "No player object"
}
```

## Message Type List

```c++
enum class MessageType {
    // Error message, would close the connection
    Error,
    // Failure message
    Failed,
    // Client message, means the message is created by a WS client
    Client,
    // Server message, means the message is created by the server
    Server,
};
```

## Action Number List

```c++
enum class Action {
    /// Game actions
    GameEnd = 1100,
    GameReady,
    GameStart,

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
    "type": "Server"
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
    "type": "Server"
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
    "type": "Server"
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
    "type": "Client",
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
    "type": "Client",
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
    "type": "Client",
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
    "type": "Client",
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
    "type": "Client",
    "data": {
        "playerId": 111,
        "role": "Admin"
    }
}

// Below would also be sent if swap happens
{
    "action": ACTION_NUMBER,
    "type": "Client",
    "data": {
        "playerId": 123,
        "role": "Normal"
    }
}
```