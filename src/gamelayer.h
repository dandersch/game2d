#pragma once

#include "SDL_thread.h"
#include "collision.h"
#include "pch.h"
#include "layer.h"
#include "player.h"
#include "renderwindow.h"
#include "camera.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/Tileset.hpp"

#define MAX_ENTITIES  100000
const int MAX_RENDER_LAYERS = 100;

class GameLayer : public Layer
{
public:
     GameLayer() : Layer("GameLayer") {}
    ~GameLayer() = default;

    virtual void OnAttach() override
    {
        tex = IMG_LoadTexture(rw->renderer, "res/character.png");
        SDL_ERROR(tex);

        tiletex = IMG_LoadTexture(rw->renderer, "res/gravetiles.png");
        SDL_ERROR(tiletex);

        // ENTITY GENERATION ///////////////////////////////////////////////////
        ents[0] = { .active = true, .freed = false,
                    .flags = (u32) EntityFlag::PLAYER_CONTROLLED |
                             (u32) EntityFlag::IS_ANIMATED |
                             (u32) EntityFlag::IS_COLLIDER,
                    .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                    .sprite{{0,0,16,32}, tex, {0,0}}};
        ents[0].collider  = { 0, 0, 16, 32};

        //memset(ents, 0, sizeof(ents));

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
                              .sprite{{0,0,16,32}, tex, {0,0}, SDL_FLIP_NONE},
                              .anim{ {{0,0,16,32}, {16,0,16,32},
                                      {32,0,16,32}, {48,0,16,32}}, 1.0f, true } };
                ents[i*j].collider  = { 0, 0, 16, 32};
            }
        }

        // TEST TILE GENERATION ////////////////////////////////////////////////
        // TODO LevelGenerator that can fill the entityarray with static tiles &
        // (items &) characters (maybe without sprites), afterwards fill
        // characters (i.e. entities with flag IS_CHARACTER or sth.) and fill
        // e.g animations of entities with CharacterType SKELETON with
        // "skeleton.tmx"
        tmx::Map charMap;
        if (!charMap.load("res/character.tmx")) { printf("charmap didnt load"); exit(1); }
        //printf("%u\n", charMap.getTilesets().at(0).getTile(1)->ID);

        // testing loading animations from .tmx (/.tsx) files
        u32 animIndex = 0;
        for (auto anim : charMap.getAnimatedTiles())
        {
            for (auto frame : anim.second.animation.frames)
            {
                //auto tileID = anim.second.animation.frames.at(0).tileID;
                auto tileID = frame.tileID;
                //printf("%u\n", charMap.getTilesets().at(0).getTile(1)->ID);
                auto pos    = charMap.getTilesets().at(0).getTile(tileID)->imagePosition;
                auto size   = charMap.getTilesets().at(0).getTile(tileID)->imageSize;
                //printf("pos: %u, %u\n", pos.x, pos.y);
                //printf("size: %u, %u\n", size.x, size.y);
                SDL_Rect bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
                ents[0].anims[animIndex].frames.push_back(bb);
                ents[0].anims[animIndex].loop = true;
                ents[0].anims[animIndex].length = 1.0f;
            }
            animIndex++;
        }
        ents[0].anim = ents[0].anims[0];

        tmx::Map map;
        if (!map.load("res/tiletest.tmx")) { printf("map didnt load"); exit(1); }

        const auto& tilecountXY = map.getTileCount();
        u32 max_tiles = tilecountXY.x * tilecountXY.y;

        u32 layercount   = 0;
        u32 testingcount = 0;
        const auto& layers = map.getLayers();
        for(const auto& layer : layers)
        {
            testingcount = 0;
            const auto& tilesets = map.getTilesets();
            const auto& ts = tilesets.at(0);
            for(const auto& tileset : tilesets)
            {
                //read out tile set properties, load textures etc...
            }

            for (const auto& t : ts.getTiles())
            {
                if (!t.objectGroup.getObjects().empty())
                {
                    const auto collAABB = t.objectGroup.getObjects().at(0).getAABB();
                }
            }

            if(layer->getType() == tmx::Layer::Type::Tile)
            {
                const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
                const auto& tiles =  tileLayer.getTiles();
                for (const auto& t : tiles)
                {
                    for (u32 i = 0; i < MAX_ENTITIES; i++) {
                        if (ents[i].freed)
                        {
                            u32 y = testingcount / tilecountXY.x;
                            u32 x = testingcount % tilecountXY.y;

                            // TODO collision box data uses pixels as units, we
                            // might want to convert this to a 0-1 range
                            auto tile = ts.getTile(t.ID);
                            if (!tile) break;
                            SDL_Rect bb = {(i32) tile->imagePosition.x,
                                           (i32) tile->imagePosition.y,
                                           (i32) tile->imageSize.x,
                                           (i32) tile->imageSize.y};

                            if (!tile->objectGroup.getObjects().empty())
                            {
                                const auto& aabb = tile->objectGroup.getObjects().at(0).getAABB();
                                ents[i].collider    = {(i32) aabb.left,  (i32) aabb.top,
                                                       (i32) aabb.width, (i32) aabb.height};
                                ents[i].flags        = (u32) EntityFlag::IS_COLLIDER;
                            }

                            ents[i].active       = true;
                            ents[i].freed        = false;
                            ents[i].renderLayer  = layercount;
                            // TODO why does this have to be 24...
                            ents[i].position     = {x * 24.f, y * 24.f, 0};
                            ents[i].tile         = { t.ID, TileType::GRASS };
                            ents[i].sprite.box   = bb;
                            ents[i].sprite.pivot = {0.5f, 0.5f};
                            ents[i].sprite.tex   = tiletex;
                            break;
                        }
                    }
                    // TODO better name / there might be an equivalent in tmx
                    testingcount++;
                }
            }
            layercount++;
        }

        // Font Test
        TTF_Init();
        TTF_Font* font        = TTF_OpenFont( "res/gothic.ttf", 40 );
        std::string text      = "Linebreaks are working.\nLook at all these perfect linebreaks.\n"
                                "This is achieved wtih TTF_RenderText_Blended_Wrapped()\n"
                                "Another line here.";

        SDL_Color textColor   = {150,160,100,230};
        SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font, text.c_str(),
                                                               textColor, 800);
        SDL_ERROR(textSurf);
        txtTex   = SDL_CreateTextureFromSurface(rw->renderer, textSurf);
        SDL_ERROR(txtTex);
    }

    virtual void OnDetach() override
    {
    }

    virtual void OnUpdate(f32 dt) override
    {
        // TODO find out if it matters if we do everything in one loop for one
        // entity vs. every "system" has its own loop
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;

            if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
                player.update(dt, ents[i]);

            // NOTE animation should probably be last after input & collision etc.
            if (ents[i].flags & (u32) EntityFlag::IS_ANIMATED)
                ents[i].sprite.box = Animator::animate(dt, ents[i].anim);
        }

        // collision checking
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            bool collided = false;
            Entity& e1 = ents[i];
            if (!e1.active) continue;
            if (!((e1.flags & (u32) EntityFlag::IS_COLLIDER) &&
                  (e1.flags & (u32) EntityFlag::PLAYER_CONTROLLED))) continue;
            for (u32 j = i; j < MAX_ENTITIES; j++)
            {
                Entity& e2 = ents[j];
                if (!e2.active) continue;
                if ((e2.flags & (u32) EntityFlag::IS_COLLIDER) && (&e1 != &e2))
                    collided = Collision::checkCollision(e1, e2);
            }
            if (!collided) ents[i].position += ents[i].movement;
        }
    }

    virtual void OnRender() override
    {
        u32 maxlayer = 0;
        for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
        {
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].renderLayer != l) continue;
                rw->render(ents[i].sprite, cam.worldToScreen(ents[i].position),
                           1.5f, ents[i].sprite.flip);

                if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
            }
            // no need to go through all renderlayers
            if (l > maxlayer) break;
        }

        // testing text
        int w,h;
        SDL_QueryTexture(txtTex, NULL, NULL, &w, &h);
        SDL_Rect dst{100,600,w,h};
        SDL_RenderCopy(rw->renderer, txtTex, NULL, &dst);
    }

    // TODO breaks when compiling w/o imgui
    virtual void OnImGuiRender() override
    {
#ifdef IMGUI
        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello World");
        //ImGui::Text("TICKS: %d", g_time);
        //ImGui::Text("ACCU: %f", accumulator);
        //ImGui::Text("DT: %f", dt);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Button("Button Test");
        ImGui::End();
#endif
    }

    virtual void OnEvent(Event& event) override
    {
        SDL_Event evn = event.evn;

        switch (evn.type) {
        case SDL_KEYDOWN:
            switch (evn.key.keysym.sym)
            {
            case SDLK_UP:
                cam.cameraRect.y -= 5;
                break;
            case SDLK_DOWN:
                cam.cameraRect.y += 5;
                break;
            case SDLK_LEFT:
                cam.cameraRect.x -= 5;
                break;
            case SDLK_RIGHT:
                cam.cameraRect.x += 5;
                break;
            }
            break;
        case SDL_MOUSEMOTION:
            break;
        case SDL_MOUSEBUTTONDOWN:
            auto click = cam.screenToWorld({evn.button.x, evn.button.y, 0});
            cam.cameraRect.x = click.x - (cam.cameraRect.w/2);
            cam.cameraRect.y = click.y - (cam.cameraRect.h/2);
            break;
        }

        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
                player.handleEvent(event, ents[i], cam);
        }
    }

private:
    SDL_Texture* tex;
    SDL_Texture* tiletex;
    SDL_Texture* txtTex;
    Player player;
    Camera cam;


public:
    // compile times blow up when this is not static and MAX_ENTITIES is large
    static Entity ents[MAX_ENTITIES];
};
