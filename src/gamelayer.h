#pragma once

#include "pch.h"
#include "layer.h"
#include "player.h"
#include "renderwindow.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"

#define MAX_ENTITIES  1000000
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

        //memset(ents, 0, sizeof(ents));
        ents[0] = { .active = true, .freed = false,
                    .flags = (u32) EntityFlag::PLAYER_CONTROLLED,
                    .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                    .sprite{{0,0,16,32}, tex, {0,0}}};

        for (u32 i = 1; i < 100; i++)
        {
            for (u32 j = 1; j < 100; j++)
            {
                ents[i*j] = { .active = true, .freed = false,
                              .flags = (u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_ANIMATED,
                              .position = {13 * i, 10 * j,0}, .orient = 3, .renderLayer = 1,
                              .sprite{{0,0,16,32}, tex, {0,0}, SDL_FLIP_VERTICAL},
                              .anim{ {{0,0,16,32}, {16,0,16,32}, {32,0,16,32}, {48,0,16,32}},
                                     1.0f, true } };
            }
        }

        // TEST TILE GENERATION
        tmx::Map map;
        if (!map.load("res/tiletest.tmx")) { printf("map didnt load"); exit(1); }

        const auto& tilecountXY = map.getTileCount();
        u32 max_tiles = tilecountXY.x * tilecountXY.y;

        u32 testingcount = 0;
        const auto& layers = map.getLayers();
        for(const auto& layer : layers)
        {
            const auto& tilesets = map.getTilesets();
            const auto& ts = tilesets.at(0);
            for(const auto& tileset : tilesets)
            {
                //read out tile set properties, load textures etc...
            }

            if(layer->getType() == tmx::Layer::Type::Tile)
            {
                const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
                const auto& tiles =  tileLayer.getTiles();
                for (const auto& t : tiles)
                {
                    testingcount++;
                    for (u32 i = 0; i < MAX_ENTITIES; i++) {
                        if (ents[i].freed)
                        {
                            // TODO convert from tileID to position
                            u32 y = testingcount / tilecountXY.x;
                            u32 x = testingcount % tilecountXY.y;

                            auto tile = ts.getTile(t.ID);
                            SDL_Rect bb = {(i32) tile->imagePosition.x,
                                           (i32) tile->imagePosition.y,
                                           (i32) tile->imageSize.x,
                                           (i32) tile->imageSize.y};

                            ents[i].active       = true;
                            ents[i].freed        = false;
                            ents[i].renderLayer  = 0;
                            // TODO why does this have to be 24...
                            ents[i].position     = {x * 24.f, y * 24.f, 0};
                            ents[i].tile         = { t.ID, TileType::GRASS };
                            ents[i].sprite.box   = bb;
                            ents[i].sprite.pivot = {0.5f, 0.5f};
                            ents[i].sprite.tex   = tiletex;
                            break;
                        }
                    }
                }
            }
        }

        // Font Test
        TTF_Init();
        TTF_Font* font        = TTF_OpenFont( "res/gothic.ttf", 40 );
        std::string text      = "Linebreaks are working.\nLook at all these perfect linebreaks.\n"
                                "This is achieved wtih TTF_RenderText_Blended_Wrapped()\n"
                                "Another line here.";

        SDL_Color textColor   = {150,160,100,230};
        //SDL_Surface* textSurf = TTF_RenderText_Solid(font, text.c_str(), textColor);
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
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].flags & (u32) EntityFlag::IS_ANIMATED)
                ents[i].sprite.box = Animator::animate(dt, ents[i].anim);
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
                rw->render(ents[i].sprite, ents[i].position, 1.5f, ents[i].sprite.flip);

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
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
                Player::handleEvent(event, ents[i]);
        }
    }

private:
    SDL_Texture* tex;
    SDL_Texture* tiletex;
    SDL_Texture* txtTex;

    // compile times blow up when this is not static and MAX_ENTITIES is large
    static Entity ents[MAX_ENTITIES]; // TODO does this zero out the array?
};
