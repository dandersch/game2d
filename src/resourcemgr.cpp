#include "resourcemgr.h"

void loadFromFile(SDL_Texture** tex, const char* file)
{
    *tex = IMG_LoadTexture(globals.rw->renderer, file);
}

// TODO ptsize hardcoded
void loadFromFile(TTF_Font** fnt, const char* file)
{
    *fnt = TTF_OpenFont(file, 50);
}
