#include "pch.h"

#include "entity.h"
#include "player.h"
#include "renderwindow.h"
#include "imguilayer.h"
#include "gamelayer.h"
#include "menulayer.h"
#include "event.h"

//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

const f32 TIME_PER_FRAME = (f32) 1/60;

bool renderImgui = false;

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

#include "sound.h"

enum Layers { LAYER_GAME, LAYER_MENU, LAYER_IMGUI, LAYER_COUNT };
// TODO maybe use a bool array for keeping track of in-/active layers, i.e.
// bool[LAYER_COUNT] = {false};

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    rw = new RenderWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderSetScale(rw->renderer, 1.f, 1.f);

    // test sound
    Sound::initAndLoadSound();

    // init layers
    layer_game_init();
    layer_menu_init();
    layer_imgui_init();

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
    layer_imgui_destroy();
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
                for (int layer = LAYER_COUNT; layer >= 0; layer--) {
                    if (evn.handled) break;
                    switch (layer) {

                    case LAYER_GAME: {
                        layer_game_handle_event(evn);
                    } break;

                    case LAYER_MENU: {
                        if (g_layer_menu_is_active) {
                            if (evn.handled) layer = 0;
                            layer_menu_handle_event(evn);
                            layer = 0; // to break out of loop
                        }
                    } break;

                    case LAYER_IMGUI: {
                        layer_imgui_handle_event(evn);
                    } break;

                    }
                }

                switch (evn.sdl.type) {
                case SDL_KEYDOWN: {
                    // TODO hardcoded
                    // toggle testmenu
                    if (evn.sdl.key.keysym.sym == SDLK_ESCAPE)
                    {
                        g_layer_menu_is_active = !g_layer_menu_is_active;
                    }
                    if (evn.sdl.key.keysym.sym == SDLK_F1)
                    {
                        renderImgui = !renderImgui;
                    }
                } break;
                case SDL_QUIT: run = false; break;
                case SDL_WINDOWEVENT: {
                    if (evn.sdl.window.type == SDL_WINDOWEVENT_CLOSE)
                        run = false;
                } break;
                }
            }

            // UPDATE LOOP /////////////////////////////////////////////////////
            for (int layer = LAYER_COUNT; layer >= 0; layer--)
            {
                // TODO hardcoded, implements 'pause' functionality
                if (g_layer_menu_is_active) break;
                switch (layer) {
                case LAYER_GAME: {
                    layer_game_update(dt);
                } break;
                }
            }
        }

        cyclesLeftOver = updateIterations;
        lastFrameTime = currentTime;

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw->renderer);

        for (int layer = 0; layer < LAYER_COUNT; layer++)
        {
            switch (layer) {

            case LAYER_GAME: {
                layer_game_render();
            } break;

            case LAYER_MENU: {
                if (!g_layer_menu_is_active) break;
                layer_menu_render();
            } break;

            }
        }

#ifdef IMGUI
        if (renderImgui)
        {
            layer_imgui_begin();
            for (int layer = 0; layer < LAYER_COUNT; layer++)
            {
                switch (layer) {
                case LAYER_GAME: {
                    layer_game_imgui_render();
                } break;
                }
            }
            layer_imgui_end();
        }
#endif
        SDL_RenderPresent(rw->renderer);
    }
}
