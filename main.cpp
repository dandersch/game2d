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

const int MAX_RENDER_LAYERS = 3;

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
    ents[0] = { .active = true, .freed = false, .flags = (u32) EntityFlag::PLAYER_CONTROLLED,
                .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                .sprite{{0,0,16,32}, tex, {0,0}} };
    ents[1] = { .active = true, .freed = false, .flags = 0,
                .position = {0,16,0}, .orient = 0, .renderLayer = 0,
                .sprite{{32,32,32,16}, tiletex, {0,0}} };
    ents[2] = { .active = true, .freed = false, .flags = 0,
                .position = {500,800,0}, .orient = 0, .renderLayer = 1,
                .sprite{{64,64,16,32}, tex, {0,0}} };

    for (u32 i = 3; i < 100; i++)
    {
        for (u32 j = 1; j < 100; j++)
        {
            ents[i*j] = { .active = true, .freed = false, .flags = 0,
                          .position = {5 * i, 10 * j,0}, .orient = 3, .renderLayer = 1,
                          .sprite{{0,0,16,32}, tex, {0,0}} };
        }
    }

    Player player(&ents[0]);

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

#ifdef IMGUI
        // DEAR IMGUI //////////////////////////////////////////////////////////
        ImGui_ImplSDL2_NewFrame(rw.window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello World");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
#endif

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw.renderer);

        for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
        {
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].renderLayer != l) continue;
                rw.render(ents[i].sprite, ents[i].position);
            }
        }


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
    SDL_Quit();

    return 0;
}
