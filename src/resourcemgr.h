#pragma once

struct game_state_t;
struct platform_api_t;
struct platform_window_t;
texture_t* resourcemgr_texture_load(const char* filename, platform_api_t* platform, platform_window_t* window);
font_t*    resourcemgr_font_load(const char* filename, platform_api_t* platform, i32 ptsize = 50);
b32        resourcemgr_free(const char* filename, game_state_t* state);
