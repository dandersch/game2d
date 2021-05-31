#pragma once

#include "pch.h"

struct Event;

extern b32 g_layer_menu_is_active;

void layer_menu_init();
void layer_menu_handle_event(Event& event);
void layer_menu_render();
