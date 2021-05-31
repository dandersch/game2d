#pragma once

#include "pch.h"

struct Event
{
    SDL_Event sdl;
    b32 handled = false;
};
