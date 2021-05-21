#pragma once

#include "entity.h"
#include "pch.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"

#include "gamelayer.h"

// TODO loading in textures on demand here

class LevelGenerator
{
public:
    LevelGenerator() {}
    ~LevelGenerator() = default;

    bool loadLevel(const std::string& file, Entity* ents, u32 max_ents)
    {
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

            if(layer->getType() == tmx::Layer::Type::Tile)
            {
                const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
                const auto& tiles     = tileLayer.getTiles();

                for (const auto& t : tiles)
                {
                    u32 y = tilecount / tilecountXY.x;
                    u32 x = tilecount % tilecountXY.y;

                    //printf("ID: %u\n", t.ID);
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
                    newEnt.sprite.tex   = GameLayer::tiletex; // TODO

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

        return true;
    }
};
