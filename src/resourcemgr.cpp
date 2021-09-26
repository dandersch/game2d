#include "resourcemgr.h"
#include "globals.h"
#include "platform.h"

// TODO load in "missing" placeholder assets
//static std::unordered_map<std::string, texture_t*> pool_textures{};
//static std::unordered_map<std::string, font_t*> pool_fonts{};
#include "memory.h"

texture_t* resourcemgr_texture_load(const char* filename, game_state_t* state)
{
    if (state->pool_textures.find(filename) != state->pool_textures.end()) // found
    {
        return state->pool_textures[filename]; // TODO don't look up twice
    }
    else // not found
    {
        state->pool_textures[filename] = platform.texture_load(state->window, filename);
        if (!state->pool_textures[filename]) return state->pool_textures["missing"];
    }

    return state->pool_textures[filename];
}

font_t* resourcemgr_font_load(const char* filename, game_state_t* state, i32 ptsize)
{
    if (state->pool_fonts.find(filename) != state->pool_fonts.end()) // found
    {
        return state->pool_fonts[filename]; // TODO don't look up twice
    }
    else // not found
    {
        state->pool_fonts[filename] = platform.font_load(filename, ptsize);
        if (!state->pool_fonts[filename]) return state->pool_fonts["missing"];
    }

    return state->pool_fonts[filename];
}

// TODO
b32 resourcemgr_free(const char* filename, game_state_t* state)
{
    if (state->pool_textures.find(filename) != state->pool_textures.end()) // found
    {
        // TODO free_resource() for all Resources
        return true;
    }
    return false; // not found
}
