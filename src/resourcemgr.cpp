#include "resourcemgr.h"

#include "platform_sdl.h" // TODO platform agnostic
#include "globals.h"

//std::unordered_map<std::string, SDL_Texture*> pool_textures{};
//std::unordered_map<std::string, TTF_Font*> pool_fonts{};

void loadFromFile(SDL_Texture** tex, const char* file)
{
    *tex = IMG_LoadTexture(globals.window->renderer, file);
}

// TODO ptsize hardcoded
void loadFromFile(TTF_Font** fnt, const char* file)
{
    *fnt = TTF_OpenFont(file, 50);
}
