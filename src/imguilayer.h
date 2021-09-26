#pragma once

#include "pch.h"
#include "platform.h"
#include "globals.h"

struct game_input_t;

#include "memory.h"
extern game_state_t* state;

// TODO needless indirections (?)
inline void layer_imgui_init()
{
    platform.imgui_init(state->window, SCREEN_WIDTH, SCREEN_HEIGHT);
}

inline void layer_imgui_destroy()
{
    platform.imgui_destroy();
}

inline void layer_imgui_handle_event(game_input_t* input)
{
    platform.imgui_event_handle(input);
}

inline void layer_imgui_begin()
{
    platform.imgui_begin(state->window);
}

inline void layer_imgui_end()
{
    platform.imgui_end();
}
