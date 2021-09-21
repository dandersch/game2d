#pragma once

#include "pch.h"

union event_t;

extern b32 g_layer_menu_is_active;

void layer_menu_init();
void layer_menu_handle_event();
void layer_menu_render();
