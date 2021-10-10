#include "resourcemgr.h"
#include "globals.h"
#include "platform.h"
#include "memory.h"

// TODO load in "missing" placeholder assets

// NOTE we use this to concatenate every filename w/ the resource folder "res/"
// this means filenames can't be longer than 256 (256-4 to be exact)
#define MAX_CHARS_FOR_FILENAME 256
static char res_folder[MAX_CHARS_FOR_FILENAME] = "res/";
static const u32 str_len = 4; // TODO hardcoded

// NOTE we use this to concatenate every filename w/ the resource folder "res/"
// this means filepaths can't be longer than 256 characters
#define MAX_CHARS_FOR_FILEPATH 256
#define RESOURCE_FOLDER "res/"
texture_t* resourcemgr_texture_load(const char* filename, game_state_t* state)
{
    char filepath[MAX_CHARS_FOR_FILEPATH] = RESOURCE_FOLDER;
    ASSERT(strlen(filename) < (MAX_CHARS_FOR_FILEPATH - strlen(RESOURCE_FOLDER)));
    strcat(filepath, filename);

    if (state->pool_textures.find(filename) != state->pool_textures.end()) // found
    {
        return state->pool_textures[filename]; // TODO don't look up twice
    }
    else // not found
    {
        state->pool_textures[filename] = platform.texture_load(state->window, filepath);
        if (!state->pool_textures[filename]) return state->pool_textures["missing"];
    }

    return state->pool_textures[filename];
}

font_t* resourcemgr_font_load(const char* filename, game_state_t* state, i32 ptsize)
{
    char filepath[MAX_CHARS_FOR_FILEPATH] = RESOURCE_FOLDER;
    ASSERT(strlen(filename) < (MAX_CHARS_FOR_FILEPATH - strlen(RESOURCE_FOLDER)));
    strcat(filepath, filename);

    if (state->pool_fonts.find(filename) != state->pool_fonts.end()) // found
    {
        return state->pool_fonts[filename]; // TODO don't look up twice
    }
    else // not found
    {
        state->pool_fonts[filename] = platform.font_load(filepath, ptsize);
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
