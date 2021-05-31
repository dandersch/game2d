#pragma once

#include "pch.h"

struct Event;

void layer_game_init();
void layer_game_handle_event(Event& event);
void layer_game_update(f32 dt);
void layer_game_render();
void layer_game_imgui_render();
