#pragma once

#include "pch.h"

enum Action
{
    ACTION_MOVE_UP    = (1 << 0),
    ACTION_MOVE_DOWN  = (1 << 1),
    ACTION_MOVE_RIGHT = (1 << 2),
    ACTION_MOVE_LEFT  = (1 << 3),
    ACTION_PICKUP     = (1 << 4),
    ACTION_ATTACK     = (1 << 5),
    ACTION_COUNT      = 6
};

namespace Input
{
    extern u32 actionState;
    //b8 actionState[ACTION_COUNT] = {0};

    void update();

} // namespace Input
