#pragma once

#include "pch.h"
#include "renderwindow.h"
#include "gamelayer.h"
#include "entity.h"
#include "resourcemgr.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"

class LevelGenerator
{
public:
    LevelGenerator() : texMgr(".png") {}
    ~LevelGenerator() = default;

    bool loadLevel(const std::string& file, Entity* ents, u32 max_ents)
    {
        // ENTITY GENERATION ///////////////////////////////////////////////////
        // TODO load in from tmx
        SDL_Texture* chartex = texMgr.get("res/character.png");
        ents[0] = { .active = true, .freed = false,
        .flags = (u32) EntityFlag::PLAYER_CONTROLLED |
        (u32) EntityFlag::IS_ANIMATED |
        (u32) EntityFlag::IS_COLLIDER,
        .position = {0,0,0}, .orient = 0, .renderLayer = 1,
        .sprite{{0,0,16,32}, chartex, {0,0}}};
        ents[0].collider  = { 0, 0, 16, 32};

        for (u32 i = 1; i < 100; i++)
        {
            for (u32 j = 1; j < 100; j++)
            {
                ents[i*j] = { .active = true, .freed = false,
                .flags = //(u32) EntityFlag::PLAYER_CONTROLLED |
                (u32) EntityFlag::IS_COLLIDER |
                (u32) EntityFlag::IS_ANIMATED,
                .position = {13 * i, 10 * j,0},
                .orient = 3, .renderLayer = 1,
                .sprite{{0,0,16,32}, chartex, {0,0}, SDL_FLIP_NONE},
                .anim{ {{0,0,16,32}, {16,0,16,32},
                        {32,0,16,32}, {48,0,16,32}}, 1.0f, true } };
                ents[i*j].collider  = { 0, 0, 16, 32};
            }
        }

        tmx::Map map;
        if (!map.load(file)) { printf("map didnt load"); return false; }

        const auto& tilecountXY  = map.getTileCount();
        u32 max_tiles            = tilecountXY.x * tilecountXY.y;
        const auto& layers       = map.getLayers();
        u32 layercount           = 0;
        u32 tilecount            = 0;

        for(const auto& layer : layers)
        {
            tilecount = 0;
            const auto& tilesets = map.getTilesets();
            const auto& ts       = tilesets.at(0);

            SDL_Texture* tiletex = texMgr.get(ts.getImagePath());

            if(layer->getType() == tmx::Layer::Type::Tile)
            {
                const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
                const auto& tiles     = tileLayer.getTiles();

                for (const auto& t : tiles)
                {
                    u32 y = tilecount / tilecountXY.x;
                    u32 x = tilecount % tilecountXY.y;

                    auto tile = ts.getTile(t.ID);
                    if (!tile) { tilecount++; continue; }
                    SDL_Rect bb = {(i32) tile->imagePosition.x,
                                   (i32) tile->imagePosition.y,
                                   (i32) tile->imageSize.x,
                                   (i32) tile->imageSize.y};

                    // CONSTRUCT ENTITY
                    Entity newEnt = {0};
                    if (!tile->objectGroup.getObjects().empty())
                    {
                        // TODO collision box data uses pixels as units, we
                        // might want to convert this to a 0-1 range
                        const auto& aabb = tile->objectGroup.getObjects().at(0).getAABB();
                        newEnt.collider  = {(i32) aabb.left,  (i32) aabb.top,
                                            (i32) aabb.width, (i32) aabb.height};
                        newEnt.flags     = (u32) EntityFlag::IS_COLLIDER;
                    }

                    newEnt.active       = true;
                    newEnt.freed        = false;
                    newEnt.renderLayer  = layercount;
                    // TODO why does this have to be 24...
                    newEnt.position     = {x * 24.f, y * 24.f, 0};
                    newEnt.tile         = { t.ID, TileType::GRASS };
                    newEnt.sprite.box   = bb;
                    newEnt.sprite.pivot = {0.5f, 0.5f};
                    newEnt.sprite.tex   = tiletex; // TODO

                    // copy new entity into array TODO slow
                    for (u32 i = 0; i < max_ents; i++)
                    {
                        if (ents[i].freed) {
                            ents[i] = newEnt;
                            break;
                        }
                    }
                    tilecount++;
                } // tile loop
            } // tilelayer
            layercount++;
        } // layer loop

        // Loading in animations
        tmx::Map charMap;
        if (!charMap.load("res/character.tmx")) { printf("charmap didnt load"); exit(1); }

        // testing loading animations from .tmx (/.tsx) files
        u32 animIndex = 0;
        for (auto anim : charMap.getAnimatedTiles())
        {
            for (auto frame : anim.second.animation.frames)
            {
                auto tileID = frame.tileID;
                auto pos    = charMap.getTilesets().at(0).getTile(tileID)->imagePosition;
                auto size   = charMap.getTilesets().at(0).getTile(tileID)->imageSize;
                SDL_Rect bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
                ents[0].anims[animIndex].frames.push_back(bb);
                ents[0].anims[animIndex].loop = true;
                ents[0].anims[animIndex].length = 1.0f;
            }
            animIndex++;
        }
        ents[0].anim = ents[0].anims[0];

        return true;
    }

private:
    ResourceManager<SDL_Texture*> texMgr;
};
