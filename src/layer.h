#pragma once

// GAMELAYER ///////////////////////////////////////////////////////////////////////////////////////
void layer_game_init();
void layer_game_handle_event();
void layer_game_update(f32 dt);
void layer_game_render();
void layer_game_imgui_render(f32 dt);

// MENULAYER ///////////////////////////////////////////////////////////////////////////////////////
void layer_menu_init();
void layer_menu_handle_event();
void layer_menu_render();

// IMGUILAYER //////////////////////////////////////////////////////////////////////////////////////
struct game_input_t;

// TODO needless indirections (?)
void layer_imgui_init();
void layer_imgui_destroy();
void layer_imgui_handle_event(game_input_t* input);
void layer_imgui_begin();
void layer_imgui_end();
