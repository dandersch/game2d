#pragma once

#include "pch.h"
#include "platform.h"
#include "globals.h"

struct game_input_t;

// TODO needless indirections (?)
inline void layer_imgui_init() { platform_imgui_init(globals.window, SCREEN_WIDTH, SCREEN_HEIGHT); }
inline void layer_imgui_destroy() { platform_imgui_destroy(); }
inline void layer_imgui_handle_event(game_input_t* input) { platform_imgui_event_handle(input); }
inline void layer_imgui_begin() { platform_imgui_begin(globals.window); }
inline void layer_imgui_end() { platform_imgui_end(); }
