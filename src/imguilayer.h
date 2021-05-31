#pragma once

#include "pch.h"

struct Event;

void layer_imgui_init();
void layer_imgui_destroy();
void layer_imgui_handle_event(Event& e);
void layer_imgui_begin();
void layer_imgui_end();
