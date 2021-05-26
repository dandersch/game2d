#include "pch.h"

#include "entity.h"
#include "player.h"
#include "layer.h"
#include "renderwindow.h"
#include "imguilayer.h"
#include "gamelayer.h"
#include "menulayer.h"

//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

const f32 TIME_PER_FRAME = (f32) 1/60;

std::vector<Layer*> layerStack;
Layer* menuLayer;
bool renderImgui = false;
#ifdef IMGUI
    ImGuiLayer* igLayer;
#endif

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

// global because we need it in the "main_loop" used for emscripten
bool run;
f32 dt;
f32 lastFrameTime;
f32 cyclesLeftOver;
f32 currentTime;
f32 updateIterations;
void main_loop();
RenderWindow* rw;

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    rw = new RenderWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetScale(rw->renderer, 1.f, 1.f); // use for zooming?

    layerStack.push_back(new GameLayer());
    menuLayer = new MenuLayer();
    menuLayer->active = false;
    layerStack.push_back(menuLayer);

#ifdef IMGUI
    igLayer = new ImGuiLayer();
    layerStack.push_back(igLayer);
#endif

    for (auto it = layerStack.begin(); it != layerStack.end(); ++it)
        (*it)->OnAttach();

    // main loop ///////////////////////////////////////////////////////////////
    run         = true;
    lastFrameTime = 0.0;
    cyclesLeftOver = 0.0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, -1, 1);
#else
    main_loop();
#endif

    // CLEANUP /////////////////////////////////////////////////////////////////
    for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
        (*it)->OnDetach();

    SDL_DestroyRenderer(rw->renderer);
    SDL_DestroyWindow(rw->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void main_loop()
{
#ifndef __EMSCRIPTEN__
    while (run)
#endif
    {
        // TIMESTEP ////////////////////////////////////////////////////////////
        currentTime = SDL_GetTicks()/ 1000.f;
        updateIterations = ((currentTime - lastFrameTime) + cyclesLeftOver);
        dt = UPDATE_INTERVAL;

        if (updateIterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
            updateIterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
        }

        while (updateIterations > UPDATE_INTERVAL) {
            updateIterations -= UPDATE_INTERVAL;

            // EVENT HANDLING //////////////////////////////////////////////////
            Event evn;
            while (SDL_PollEvent(&evn.sdl))
            {
                for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
                {
                    if (!(*it)->active) continue;
                    if (evn.handled) break;
                    (*it)->OnEvent(evn);

                    // TODO hardcoded, implements 'pause' functionality
                    if (menuLayer->active)
                    {
                        if (evn.handled) break;
                        menuLayer->OnEvent(evn);
                        break;
                    }
                }

                switch (evn.sdl.type) {
                    case SDL_KEYDOWN:
                        // TODO hardcoded
                        // toggle testmenu
                        if (evn.sdl.key.keysym.sym == SDLK_ESCAPE)
                        {
                            menuLayer->active = !menuLayer->active;
                        }
                        if (evn.sdl.key.keysym.sym == SDLK_F1)
                        {
                            renderImgui = !renderImgui;
                        }
                        break;
                    case SDL_QUIT: run = false;
                        break;
                    case SDL_WINDOWEVENT:
                        if (evn.sdl.window.type == SDL_WINDOWEVENT_CLOSE)
                            run = false;
                        break;
                }
            }

            // UPDATE LOOP /////////////////////////////////////////////////////
            // TODO update back to front or front to back?
            for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
            {
                if (!(*it)->active) continue;
                (*it)->OnUpdate(dt);

                // TODO hardcoded, implements 'pause' functionality
                if (menuLayer->active)
                {
                    menuLayer->OnUpdate(dt);
                    break;
                }
            }
        }

        cyclesLeftOver = updateIterations;
        lastFrameTime = currentTime;

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw->renderer);

        for (auto it = layerStack.begin(); it != layerStack.end(); ++it)
        {
            if (!(*it)->active) continue;
            (*it)->OnRender();
        }

#ifdef IMGUI
        if (renderImgui)
        {
            igLayer->Begin();
            for (auto l : layerStack) l->OnImGuiRender();
            igLayer->End();
        }
#endif
        SDL_RenderPresent(rw->renderer);
    }

}
