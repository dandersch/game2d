#pragma once

#ifdef false

enum class SpriteID
{
    NONE,
    TILE_GRASS,
    ITEM_SWORD,
    ITEM_BOW,
    SKELE_IDLE_UP,
    SKELE_IDLE_DOWN,
    SKELE_IDLE_LEFT,
    SKELE_IDLE_RIGHT
};

struct Sprite
{
    SDL_Texture* tex;
    SDL_Rect     box;
    v2f          pivot; // SDL positioning is based on top-left coordinate
};

Sprite sprites[];

#endif
