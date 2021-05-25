#pragma once

#include "pch.h"
#include "renderwindow.h"
#include "gamelayer.h"
#include "entity.h"
#include "resourcemgr.h"
#include "rewind.h"
#include "command.h"
#include "entitymgr.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"

class LevelGenerator
{
public:
    LevelGenerator() : texMgr(".png") {}
    ~LevelGenerator() = default;

    // TODO internal
    // TODO needed?
    const tmx::Tileset& getTilesetByName(const std::string& name,
                                         const std::vector<tmx::Tileset, std::allocator<tmx::Tileset>> tilesets)
    {
        for (auto& ts : tilesets)
        {
            if (ts.getName() == name)
                return ts;
        }
        ASSERT(false); // TODO
    };

    // TEST TILE GENERATION ////////////////////////////////////////////////
    // TODO LevelGenerator that can fill the entityarray with static tiles &
    // (items &) characters (maybe without sprites), afterwards fill characters
    // (i.e. entities with flag IS_CHARACTER or sth.) and fill e.g animations of
    // entities with CharacterType SKELETON with "skeleton.tmx"
    bool loadLevel(const std::string& file, Entity* ents, u32 max_ents)
    {
        // ENTITY GENERATION ///////////////////////////////////////////////////
        // TODO load in from tmx
        SDL_Texture* chartex = texMgr.get("res/character.png");
        Entity ent0 = { .active = true, .freed = false,
                        .flags = (u32) EntityFlag::PLAYER_CONTROLLED |
                             (u32) EntityFlag::IS_ANIMATED |
                             (u32) EntityFlag::IS_REWINDABLE |
                             (u32) EntityFlag::IS_COLLIDER,
                    .position = {600,600,0}, .orient = 0, .renderLayer = 1,
                    .sprite{{0,0,16,32}, chartex, {0.5f,0.75f}},
                    .collider  = { 0, 0, 16, 32}};
        Rewind::initializeFrames(ent0);
        CommandProcessor::initialize(ent0);
        EntityMgr::copyEntity(ent0);

        // stress test
        for (u32 i = 1; i < 10; i++)
        {
            for (u32 j = 1; j < 10; j++)
            {
                Entity ent({ .active = true, .freed = false,
                .flags = // (u32) EntityFlag::PLAYER_CONTROLLED |
                (u32) EntityFlag::IS_COLLIDER |
                (u32) EntityFlag::IS_ITEM |
                (u32) EntityFlag::IS_ANIMATED,
                .position = {13 * i, 10 * j,0},
                .orient = 3, .renderLayer = 1,
                .sprite{{0,0,16,32}, chartex, {0,0}, SDL_FLIP_NONE},
                .anim{ {{0,0,16,32}, {16,0,16,32},
                        {32,0,16,32}, {48,0,16,32}}, 1.0f, true },
                .collider  = { 0, 0, 16, 32}});
                // Rewind::initializeFrames(ent);
                // CommandProcessor::initialize(ent);
                EntityMgr::copyEntity(ent);

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
            auto& ts = tilesets.at(0); // TODO

            SDL_Texture* tiletex = texMgr.get(ts.getImagePath());

            // for items & characters
            if(layer->getType() == tmx::Layer::Type::Object)
            {
                const auto& objs = layer->getLayerAs<tmx::ObjectGroup>().getObjects();
                printf("%zu\n", objs.size());
                auto& ts       = tilesets.at(1); //TODO hardcoded
                for (const auto& o : objs)
                {
                    const std::string& type = o.getType();
                    const std::string& name = o.getName();
                    Entity newEnt = {0};
                    newEnt.active       = true;
                    newEnt.freed        = false;
                    newEnt.renderLayer  = layercount;

                    // get tileset
                    auto t = ts.getTile(o.getTileID());

                    // to create the spritebox
                    SDL_Rect spritebox = {0};
                    ASSERT(t != nullptr);
                    spritebox = { (i32) t->imagePosition.x, (i32) t->imagePosition.y,
                                  (i32) t->imageSize.x,     (i32) t->imageSize.y };

                    // TODO load in anims in here
                    if (type == "Character")
                    {
                        // TODO charID
                        newEnt.sprite.box   = spritebox;
                        newEnt.sprite.pivot = {0.5f, 0.5f};
                        // TODO why -24
                        newEnt.setPivPos( {o.getPosition().x,
                                           o.getPosition().y - 24, 0});
                        newEnt.sprite.tex   = chartex; // TODO
                        const auto& aabb    = o.getAABB();
                        newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                               (i32) aabb.width, (i32) aabb.height};
                        newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;

                    } else if (type == "Item") {
                        newEnt.sprite.box   = spritebox;
                        newEnt.sprite.pivot = {0.5f, 0.75f};
                        // TODO why -24
                        newEnt.setPivPos( {o.getPosition().x,
                                           o.getPosition().y - 24, 0});
                        newEnt.sprite.tex   = chartex; // TODO
                        const auto& aabb    = o.getAABB();
                        newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                               (i32) aabb.width, (i32) aabb.height};
                        newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;
                        newEnt.flags       |= (u32) EntityFlag::IS_ITEM;
                        newEnt.flags       |= (u32) EntityFlag::IS_REWINDABLE;
                        Rewind::initializeFrames(newEnt);
                    }

                    // copy new entity into array TODO slow
                    EntityMgr::copyEntity(newEnt);
                } // object loop
            } // objectlayer

            // for static tiles w/ and w/o colliders
            if(layer->getType() == tmx::Layer::Type::Tile)
            {
                const auto& tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();

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
                    //newEnt.position     = ;
                    newEnt.tile         = { t.ID, TileType::GRASS };
                    newEnt.sprite.box   = bb;
                    newEnt.sprite.pivot = {0.5f, 0.5f};
                    newEnt.sprite.tex   = tiletex; // TODO
                    newEnt.setPivPos({x * 16.f, y * 16.f, 0});

                    // copy new entity into array TODO slow
                    EntityMgr::copyEntity(newEnt);
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
        auto& ent = EntityMgr::getArray()[0];
        for (auto anim : charMap.getAnimatedTiles())
        {
            for (auto frame : anim.second.animation.frames)
            {
                auto tileID = frame.tileID;
                auto pos    = charMap.getTilesets().at(0).getTile(tileID)->imagePosition;
                auto size   = charMap.getTilesets().at(0).getTile(tileID)->imageSize;
                SDL_Rect bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
                ent.anims[animIndex].frames.push_back(bb);
                ent.anims[animIndex].loop = true;
                ent.anims[animIndex].length = 1.0f;
            }
            animIndex++;
        }
        ent.anim = ent.anims[0];

        return true;
    }

private:
    ResourceManager<SDL_Texture*> texMgr;
};
