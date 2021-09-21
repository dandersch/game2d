#include "resourcemgr.h"
#include "globals.h"
#include "platform.h"

// TODO load in "missing" placeholder assets
static std::unordered_map<std::string, texture_t*> pool_textures{};
static std::unordered_map<std::string, font_t*> pool_fonts{};

texture_t* resourcemgr_texture_load(const char* filename)
{
    if (pool_textures.find(filename) != pool_textures.end()) // found
    {
        return pool_textures[filename]; // TODO don't look up twice
    }
    else // not found
    {
        pool_textures[filename] = platform_texture_load(globals.window, filename);
        if (!pool_textures[filename]) return pool_textures["missing"];
    }

    return pool_textures[filename];
}

font_t* resourcemgr_font_load(const char* filename, i32 ptsize)
{
    if (pool_fonts.find(filename) != pool_fonts.end()) // found
    {
        return pool_fonts[filename]; // TODO don't look up twice
    }
    else // not found
    {
        pool_fonts[filename] = platform_font_load(filename, ptsize);
        if (!pool_fonts[filename]) return pool_fonts["missing"];
    }

    return pool_fonts[filename];
}

// TODO
b32 resourcemgr_free(const char* filename)
{
    if (pool_textures.find(filename) != pool_textures.end()) // found
    {
        // TODO free_resource() for all Resources
        return true;
    }
    return false; // not found
}
