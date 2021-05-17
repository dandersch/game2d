#include "SDL_pixels.h"
#include "SDL_timer.h"
#include "SDL_ttf.h"
#include "pch.h"

#include "base.h"
#include "renderwindow.h"
#include "entity.h"
#include "player.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT  960
#define MAX_ENTITIES  10000
//
//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

const int MAX_RENDER_LAYERS = 100;
const f32 TIME_PER_FRAME = (f32) 1/60;

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    RenderWindow rw(SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Texture* tex = IMG_LoadTexture(rw.renderer, "res/character.png");
    SDL_ERROR(tex);

    SDL_Texture* tiletex = IMG_LoadTexture(rw.renderer, "res/gravetiles.png");
    SDL_ERROR(tiletex);

    Entity ents[MAX_ENTITIES] = {0}; // TODO does this zero out the array?
    // memset(ents, 0, sizeof(ents));
    ents[0] = { .active = true, .freed = false,
                .flags = (u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_ANIMATED,
                .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                .sprite{{0,0,16,32}, tex, {0,0}},
                .anim{ {{0,0,16,32}, {16,0,16,32}, {32,0,16,32}}, 2.0f, true } };
    ents[1] = { .active = true, .freed = false, .flags = 0,
                .position = {0,16,0}, .orient = 0, .renderLayer = 0,
                .sprite{{32,32,32,16}, tiletex, {0,0}},
                .anim = {0} };
    ents[2] = { .active = true, .freed = false, .flags = 0,
                .position = {500,800,0}, .orient = 0, .renderLayer = 1,
                .sprite{{64,64,16,32}, tex, {0,0}}, .anim = {0} };

    for (u32 i = 3; i < 100; i++)
    {
        for (u32 j = 1; j < 100; j++)
        {
            ents[i*j] = { .active = true, .freed = false,
                          .flags = (u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_ANIMATED,
                          .position = {13 * i, 10 * j,0}, .orient = 3, .renderLayer = 1,
                          .sprite{{0,0,16,32}, tex, {0,0}},
                          .anim{ {{0,0,16,32}, {16,0,16,32}, {32,0,16,32}}, 2.0f, true } };
                          //.anim = {0} };
        }
    }

    // Font Test
    TTF_Init();
    TTF_Font* font        = TTF_OpenFont( "res/gothic.ttf", 40 );
    std::string text      = "Ich bin Diego. Mich interessiert nicht wer du bist";
    SDL_Color textColor   = {150,160,100,230};
    //SDL_Surface* textSurf = TTF_RenderText_Solid(font, text.c_str(), textColor);
    SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font, text.c_str(),
                                                           textColor, 210);
    SDL_ERROR(textSurf);
    SDL_Texture* txtTex   = SDL_CreateTextureFromSurface(rw.renderer, textSurf);
    SDL_ERROR(txtTex);

#ifdef IMGUI
    // IMGUI SETUP /////////////////////////////////////////////////////////////
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiSDL::Initialize(rw.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
    // don't call an init function, but all of them require a rendering api
    // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
    // eventhandling. We expose the internal function below to circumvent that.
    ImGui_ImplSDL2_Init(rw.window);
    ImGui::StyleColorsDark();
#endif

    // LOAD TEXTURE ////////////////////////////////////////////////////////////

    // main loop ///////////////////////////////////////////////////////////////
    bool run = true;
    u32 time = 0;
    f32 dt = 0;
    f32 accumulator = 0;
    while (run)
    {
        // EVENT HANDLING //////////////////////////////////////////////////////
        SDL_Event evn;
        while (SDL_PollEvent(&evn))
        {
#ifdef IMGUI
            ImGui_ImplSDL2_ProcessEvent(&evn);
#endif

            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
                  Player::handleEvent(evn, io, ents[i]); // TODO remove imgui dependency
            }

            switch (evn.type) {
            case SDL_QUIT: run = false;
                break;
            case SDL_WINDOWEVENT:
                if (evn.window.type == SDL_WINDOWEVENT_CLOSE) run = false;
                break;
            }
        }

        dt = (SDL_GetTicks() - time) / 1000.f;
        time = SDL_GetTicks();
        accumulator += (time/1000.f);

        // TODO not working
        //while (accumulator > TIME_PER_FRAME) {
        //    accumulator -= TIME_PER_FRAME;
        //}

        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].flags & (u32) EntityFlag::IS_ANIMATED)
                ents[i].sprite.box = Animator::animate(dt, ents[i].anim);
        }

#ifdef IMGUI
        // DEAR IMGUI //////////////////////////////////////////////////////////
        ImGui_ImplSDL2_NewFrame(rw.window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello World");
        ImGui::Text("TICKS: %d", time);
        ImGui::Text("ACCU: %f", accumulator);
        ImGui::Text("DT: %f", dt);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
#endif

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw.renderer);

        u32 maxlayer = 0;
        for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
        {
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].renderLayer != l) continue;
                rw.render(ents[i].sprite, ents[i].position);

                if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
            }
            // no need to go through all renderlayers
            if (l > maxlayer) break;
        }

        int w,h;
        SDL_QueryTexture(txtTex, NULL, NULL, &w, &h);
        SDL_Rect dst{100,300,w,h};
        SDL_RenderCopy(rw.renderer, txtTex, NULL, &dst);

#ifdef IMGUI
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
#endif
        SDL_RenderPresent(rw.renderer);
    }

    // CLEANUP /////////////////////////////////////////////////////////////////
#ifdef IMGUI
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
#endif
    SDL_DestroyRenderer(rw.renderer);
    SDL_DestroyWindow(rw.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
