#pragma once

#include "platform.h"

//extern b32 g_layer_menu_is_active;

struct game_state_t;
void layer_menu_init();
void layer_menu_handle_event();
void layer_menu_render();

#include <functional> // TODO move this into the cpp once game_state is opaque to the platform layer
struct Button
{
    const char* label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    rect_t     box;
    // TODO maybe use 1 tex w/ an array of rects
    texture_t* tex[COUNT];
    texture_t* text_texture;
    rect_t     text_box;
    std::function<void(void)> callback;
};
