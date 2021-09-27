#pragma once

struct game_state_t;
void* resourcemgr_texture_load(const char* filename, game_state_t* state);
void* resourcemgr_font_load(const char* filename, game_state_t* state, i32 ptsize = 50);
b32   resourcemgr_free(const char* filename, game_state_t* state);
