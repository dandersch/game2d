#include <SDL.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL_video.h>

#include "Box2D/Box2D.h"

#ifdef IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"
#endif

#include <stdio.h>

#include "base.h"
#include "renderwindow.h"
#include "entity.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT  960
#define MAX_ENTITIES  1000
//
//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    RenderWindow rw(SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Texture* tex = IMG_LoadTexture(rw.renderer, "res/character.png");
    SDL_ERROR(tex);

    Entity ents[MAX_ENTITIES] = {0}; // TODO does this zero out the array?
    // memset(ents, 0, sizeof(ents));
    ents[0] = { .active = true, .freed = false, .flags = 1,
                .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                .sprite{{0,0,32,32}, tex, {0,0}} };
    ents[1] = { .active = true, .freed = false, .flags = 1,
                .position = {500,800,0}, .orient = 0, .renderLayer = 1,
                .sprite{{2,2,16,32}, tex, {0,0}} };

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

        for (int i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            rw.render(ents[i].sprite, ents[i].position);
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
