#include "pch.h"

#include "entity.h"
#include "player.h"
#include "imguilayer.h"
#include "gamelayer.h"
#include "menulayer.h"
#include "globals.h"
#include "platform.h"

#include "memory.h"

b32 render_imgui = false;

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

enum Layers { LAYER_GAME, LAYER_MENU, LAYER_IMGUI, LAYER_COUNT };
// TODO maybe use a bool array for keeping track of in-/active layers, i.e.
// bool[LAYER_COUNT] = {false};

game_state_t* state = nullptr;

// TODO doesn't get called anymore
extern "C" void game_state_update(game_state_t* game_state)
{
    state    = game_state;
    platform = state->platform; // TODO maybe just use state.platform...
}

extern "C" b32 game_init(game_state_t* game_state)
{
    state = game_state;

    // window setup
    //globals.window = platform.window_open("hello game", SCREEN_WIDTH, SCREEN_HEIGHT);
    state->window = platform.window_open("hello game", SCREEN_WIDTH, SCREEN_HEIGHT);

    // init layers
    layer_game_init();      // TODO level load
    layer_menu_init();
    layer_imgui_init();

    return true; // TODO error handling
}

extern "C" b32 game_quit()
{
    // CLEANUP /////////////////////////////////////////////////////////////////
    layer_imgui_destroy();
    platform.window_close(state->window);
    platform.quit();
    return true;
}

extern "C" void game_main_loop()
{
    // TIMESTEP ////////////////////////////////////////////////////////////
    f32 curr_time = platform.ticks() / 1000.f;
    f32 update_iterations = ((curr_time - state->last_frame_time) + state->cycles_left_over);
    state->dt = UPDATE_INTERVAL;

    if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
        update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
    }

    while (update_iterations > UPDATE_INTERVAL) {
        update_iterations -= UPDATE_INTERVAL;

        // EVENT HANDLING //////////////////////////////////////////////////
        platform.event_loop(&state->game_input);

        // if (globals.game_input.quit_requested) globals.game_running = false;

        // toggle testmenu TODO hardcoded
        if (input_pressed(state->game_input.keyboard.keys['\e']))
            state->g_layer_menu_is_active = !state->g_layer_menu_is_active;

        if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
        {
            printf("LMB pressed at: ");
            printf("%i ", state->game_input.mouse.pos.x);
            printf("%i\n", state->game_input.mouse.pos.y);
        }

        if (state->game_input.keyboard.f_key_pressed[1])
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
                    if (state->g_layer_menu_is_active) {
                        // if (evn.handled) layer = 0;
                        layer_menu_handle_event();
                        layer = 0; // to break out of loop
                    }
                } break;

                case LAYER_IMGUI:
                {
                    layer_imgui_handle_event(&state->game_input);
                } break;

            }
        }

        // UPDATE LOOP /////////////////////////////////////////////////////
        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            // TODO hardcoded, implements 'pause' functionality
            if (state->g_layer_menu_is_active) break;
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_update(state->dt);
                } break;
            }
        }
    }

    state->cycles_left_over = update_iterations;
    state->last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////
    platform.render_clear(state->window);

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
                if (!state->g_layer_menu_is_active) break;
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
    platform.render_present(state->window);
}
