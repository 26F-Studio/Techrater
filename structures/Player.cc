//
// Created by g29624 on 2022/8/30.
//

#include <structures/Player.h>

using namespace drogon;
using namespace std;
using namespace techmino::structures;

Player::Player(
        int64_t userId,
        Role role,
        State state,
        Type type
) : PlayerBase(userId),
    role(role),
    state(state),
    type(type) {}
