#include <SDL.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL_video.h>

#include "glm/glm.hpp"
#include "Box2D/Box2D.h"
#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"

#include <stdio.h>

#include "base.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT  980

int main(int argc, char* args[])
{
    // SDL SETUP /////////////////////////////////////////////////////////////
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { printf("%s\n", SDL_GetError()); return 1; }
    SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer",
		                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // IMGUI SETUP ///////////////////////////////////////////////////////////
    ImGui::CreateContext();
    ImGuiSDL::Initialize(rend, SCREEN_WIDTH, SCREEN_HEIGHT);
    ImGui::StyleColorsDark();
    // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
    // don't call an init function, but all of them require a rendering api
    // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
    // eventhandling. We expose the internal function below to circumvent that.
    ImGui_ImplSDL2_Init(window);

    // main loop //////////////////////////////////////////////////////////////
    while (true)
    {
	// EVENT HANDLING /////////////////////////////////////////////////////
    	SDL_Event evn;
        while (SDL_PollEvent(&evn))
        {
            ImGui_ImplSDL2_ProcessEvent(&evn);
            switch (evn.type)
            {
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    exit(0);
                    break;
                case SDL_WINDOWEVENT:
                    if (evn.window.type == SDL_WINDOWEVENT_CLOSE)
                    {
                        SDL_DestroyWindow(window);
                        SDL_Quit();
                        exit(0);
                    }
                    break;
            }
        }

        // DEAR IMGUI ///////////////////////////////////////////////////////////
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	ImGui::Begin("Hello World");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

	// RENDERING /////////////////////////////////////////////////////////////
	SDL_RenderClear(rend);
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	SDL_RenderPresent(rend);
    }

    // CLEANUP ///////////////////////////////////////////////////////////////////
    ImGuiSDL::Deinitialize();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    ImGui::DestroyContext();
    SDL_Quit();

    return 0;
}

/*
entity
{
    flags;
    active;
    position;
    rotation;
    scale;
    sprite;
    sfx;
    anim;
    enemytype;
    physicstype;
    // ...
}

entities[MAX_AMOUNT_OF_ENTITIES];
entityCount;
entityIdx;

entities[getIdxForNewEntity()] = { .position = {},
                   .sfx      =   ,
                   .anim     =   ,
                   .sprite   =   ; }

int getIdxForNewEntity()
{
    if (entityIdx == entityCount) return entityIdx;
    else
    {
        int temp  = entityIdx;
        entityIdx = entityCount;
        return tmp;
    }
}

for (ent : entites)
{
    if (ent.flags & PLAYER_CONTROLLED)
    {
        player.control(ent);
    }

    if (ent.flags & ENEMY)
    {
        aicontroller.update(ent);
    }

    if (ent.flags & TIME_REWINDABLE)
    {
        timerewinder.record(ent);
    }

    if (ent.flags & PHYSICS_BODY)
    {
        physics.update(ent);
    }
}
*/
