#include "pch.h"

#include "entity.h"
#include "player.h"
#include "renderwindow.h"
#include "imguilayer.h"
#include "gamelayer.h"
#include "menulayer.h"
#include "event.h"
#include "globals.h"

#include "platform.h"

b32 render_imgui = false;

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

void main_loop();

enum Layers { LAYER_GAME, /*LAYER_MENU, LAYER_IMGUI,*/ LAYER_COUNT };
// TODO maybe use a bool array for keeping track of in-/active layers, i.e.
// bool[LAYER_COUNT] = {false};

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    //globals.rw = new RenderWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
    globals.window = platform_window_open("Hello SDL", SCREEN_WIDTH, SCREEN_HEIGHT);

    // init layers
    layer_game_init();
    //layer_menu_init();
    layer_imgui_init();

    // main loop ///////////////////////////////////////////////////////////////
    globals.game_running   = true;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(main_loop, -1, 1);
#else
    while (globals.game_running)
        main_loop();
#endif


    // CLEANUP /////////////////////////////////////////////////////////////////
    layer_imgui_destroy();

    // TODO should happen in platform code
    //SDL_DestroyRenderer(window->renderer);
    //SDL_DestroyWindow(window->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void main_loop()
{
    // TIMESTEP ////////////////////////////////////////////////////////////
    f32 curr_time = SDL_GetTicks()/ 1000.f;
    f32 update_iterations = ((curr_time - globals.last_frame_time) + globals.cycles_left_over);
    globals.dt = UPDATE_INTERVAL;

    if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
        update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
    }

    while (update_iterations > UPDATE_INTERVAL) {
        update_iterations -= UPDATE_INTERVAL;

        // EVENT HANDLING //////////////////////////////////////////////////
        Event evn;
        while (SDL_PollEvent(&evn.sdl))
        {
            for (int layer = LAYER_COUNT; layer >= 0; layer--) {
                if (evn.handled) break;
                switch (layer)
                {
                    case LAYER_GAME:
                    {
                        layer_game_handle_event(evn);
                    } break;

                    /*
                    case LAYER_MENU:
                    {
                        if (g_layer_menu_is_active) {
                            if (evn.handled) layer = 0;
                            layer_menu_handle_event(evn);
                            layer = 0; // to break out of loop
                        }
                    } break;

                    case LAYER_IMGUI:
                    {
                        layer_imgui_handle_event(evn);
                    } break;
                    */

                }
            }

            switch (evn.sdl.type)
            {
                case SDL_KEYDOWN:
                {
                    // TODO hardcoded
                    // toggle testmenu
                    if (evn.sdl.key.keysym.sym == SDLK_ESCAPE)
                    {
                        //g_layer_menu_is_active = !g_layer_menu_is_active;
                    }
                    if (evn.sdl.key.keysym.sym == SDLK_F1)
                    {
                        render_imgui = !render_imgui;
                    }
                } break;

                case SDL_QUIT:
                {
                    globals.game_running = false;
                } break;

                case SDL_WINDOWEVENT:
                {
                    if (evn.sdl.window.type == SDL_WINDOWEVENT_CLOSE)
                        globals.game_running = false;
                } break;
            }
        }

        // UPDATE LOOP /////////////////////////////////////////////////////
        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            // TODO hardcoded, implements 'pause' functionality
            //if (g_layer_menu_is_active) break;
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_update(globals.dt);
                } break;
            }
        }
    }

    globals.cycles_left_over = update_iterations;
    globals.last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////
    //SDL_RenderClear(globals.rw->renderer);
    platform_render_clear(globals.window);

    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        switch (layer) {

        case LAYER_GAME: {
            layer_game_render();
        } break;

        /*
        case LAYER_MENU: {
            if (!g_layer_menu_is_active) break;
            layer_menu_render();
        } break;
        */

        }
    }

#ifdef IMGUI
    if (render_imgui)
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
    platform_render_present(globals.window);
}
