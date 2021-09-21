#include "pch.h"

#include "entity.h"
#include "player.h"
#include "imguilayer.h"
#include "gamelayer.h"
#include "menulayer.h"
#include "globals.h"
#include "platform.h"

b32 render_imgui = false;

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

void main_loop();

enum Layers { LAYER_GAME, LAYER_MENU, LAYER_IMGUI, LAYER_COUNT };
// TODO maybe use a bool array for keeping track of in-/active layers, i.e.
// bool[LAYER_COUNT] = {false};

// TODO move entry point into platform code and call game_main (i.e. this main) from there
int main(int argc, char* args[])
{
    // WINDOW SETUP ///////////////////////////////////////////////////////////////
    globals.window = platform_window_open("Hello Game", SCREEN_WIDTH, SCREEN_HEIGHT);

    // init layers
    layer_game_init();
    layer_menu_init();
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
    platform_window_close(globals.window);
    platform_quit();

    return 0;
}

void main_loop()
{
    // TIMESTEP ////////////////////////////////////////////////////////////
    f32 curr_time = platform_ticks() / 1000.f;
    f32 update_iterations = ((curr_time - globals.last_frame_time) + globals.cycles_left_over);
    globals.dt = UPDATE_INTERVAL;

    if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
        update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
    }

    while (update_iterations > UPDATE_INTERVAL) {
        update_iterations -= UPDATE_INTERVAL;

        // EVENT HANDLING //////////////////////////////////////////////////
        platform_event_loop(&globals.game_input);

        if (globals.game_input.quit_requested) globals.game_running = false;

        // toggle testmenu TODO hardcoded
        if (input_pressed(globals.game_input.keyboard.keys['\e']))
            g_layer_menu_is_active = !g_layer_menu_is_active;

        if (input_pressed(globals.game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
        {
            printf("LMB pressed at: ");
            printf("%i ", globals.game_input.mouse.pos.x);
            printf("%i\n", globals.game_input.mouse.pos.y);
        }

        if (globals.game_input.keyboard.f_key_pressed[1])
        {
            printf("f1 pressed\n");
            render_imgui = !render_imgui;
        }


        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            //if (evn.handled) break;
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_handle_event();
                } break;

                case LAYER_MENU:
                {
                    if (g_layer_menu_is_active) {
                        // if (evn.handled) layer = 0;
                        layer_menu_handle_event();
                        layer = 0; // to break out of loop
                    }
                } break;

                case LAYER_IMGUI:
                {
                    layer_imgui_handle_event(&globals.game_input);
                } break;

            }
        }

        // UPDATE LOOP /////////////////////////////////////////////////////
        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            // TODO hardcoded, implements 'pause' functionality
            if (g_layer_menu_is_active) break;
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
    platform_render_clear(globals.window);

    for (int layer = 0; layer < LAYER_COUNT; layer++)
    {
        switch (layer)
        {
            case LAYER_GAME:
            {
                layer_game_render();
            } break;

            case LAYER_MENU:
            {
                if (!g_layer_menu_is_active) break;
                layer_menu_render();
            } break;
        }
    }

#ifdef IMGUI
    if (render_imgui)
    {
        layer_imgui_begin();
        for (int layer = 0; layer < LAYER_COUNT; layer++)
        {
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_imgui_render();
                } break;
            }
        }
        layer_imgui_end();
    }
#endif
    platform_render_present(globals.window);
}
