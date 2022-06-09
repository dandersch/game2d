#pragma once

struct game_state_t;
struct platform_api_t;
struct platform_window_t;
texture_t* resourcemgr_texture_load(const char* filename, renderer_api_t* renderer);
b32        resourcemgr_free(const char* filename, game_state_t* state);
