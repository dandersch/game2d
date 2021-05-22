#include "pch.h"

#include "entity.h"
#include "player.h"
#include "layer.h"
#include "renderwindow.h"
#include "imguilayer.h"
#include "gamelayer.h"

//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

const f32 TIME_PER_FRAME = (f32) 1/60;

std::vector<Layer*> layerStack;
#ifdef IMGUI
    ImGuiLayer* igLayer;
#endif

// global because we need it in the "main_loop" used for emscripten
RenderWindow* rw;
u32 g_time;
f32 dt;
f32 accumulator;
bool run;
void main_loop();

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    rw = new RenderWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetScale(rw->renderer, 1.f, 1.f); // use for zooming?

    layerStack.push_back(new GameLayer());

#ifdef IMGUI
    igLayer = new ImGuiLayer();
    layerStack.push_back(igLayer);
#endif

    for (auto it = layerStack.begin(); it != layerStack.end(); ++it)
        (*it)->OnAttach();

    // main loop ///////////////////////////////////////////////////////////////
    g_time      = SDL_GetTicks();
    dt          = 0.f;
    accumulator = 0;
    run         = true;

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
        dt = (SDL_GetTicks() - g_time) / 1000.f;
        g_time = SDL_GetTicks();
        accumulator += (g_time/1000.f);

        dt = 1/60.f; // TODO

        // TODO not working
        //while (accumulator > TIME_PER_FRAME) {
        //    accumulator -= TIME_PER_FRAME;
        //}

        // EVENT HANDLING //////////////////////////////////////////////////////
        Event evn;
        while (SDL_PollEvent(&evn.evn))
        {
            for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
            {
                if (evn.handled) break; // TODO doesnt work
                (*it)->OnEvent(evn);
            }

            switch (evn.evn.type) {
            case SDL_QUIT: run = false;
                break;
            case SDL_WINDOWEVENT:
                if (evn.evn.window.type == SDL_WINDOWEVENT_CLOSE) run = false;
                break;
            }
        }

        // UPDATE LOOP /////////////////////////////////////////////////////////
        // TODO update back to front or front to back?
        for (auto it = layerStack.begin(); it != layerStack.end(); ++it)
            (*it)->OnUpdate(dt);

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw->renderer);

        for (auto it = layerStack.begin(); it != layerStack.end(); ++it)
            (*it)->OnRender();

#ifdef IMGUI
        igLayer->Begin(); // TODO can be put further down?
        for (auto l : layerStack) l->OnImGuiRender();
        igLayer->End();
#endif
        SDL_RenderPresent(rw->renderer);
    }

}
