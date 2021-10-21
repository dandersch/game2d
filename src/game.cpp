#include "game.h"

#include "memory.h"
// TODO move this somewhere else ///////////////////////////////////////////////////////////////////
struct platform_window_t;

typedef void (*button_callback_fn)(game_state_t*);
struct Button
{
    const char* label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    rect_t     box;
    // TODO maybe use 1 tex w/ an array of rects
    texture_t*  tex[COUNT];
    texture_t*  text_texture;
    rect_t     text_box;
    //std::function<void(game_state_t*)> callback;
    button_callback_fn callback;
};
#define MENU_BUTTON_COUNT 3
struct game_state_t
{
    b32 initialized = true;            // used by game
    //platform_api_t platform;         // unused
    platform_window_t* window;         // used by game, layer, resourcemgr
    b32 game_running = true;           // used by game

    Entity ents[MAX_ENTITIES] = {};    // used by entity, game, layer
    u32    temp_count         = 0;     // used by entity
    Tile   tiles[MAX_TILES]   = {};    // used by entity, game
    u32    tile_count         = 0;     // used by entity

    game_input_t game_input;           // used by input, layer
    u32 actionState;                   // used by input, rewind, player

    f32 last_frame_time;               // global because we need it in the
    f32 cycles_left_over;              // "main_loop" used for emscripten

    Camera cam = {};                   // used by game, layer
    bool debugDraw;                    // used by layer
    Entity* focusedEntity;             // used by layer
    rect_t focusArrow = {64,32,16,32}; // used by layer TODO hardcoded

    // commandprocessor
    u32 cmdIdx = 0;            // used by rewind, layer

    // reset
    bool isRewinding;          // used by rewind, layer
    f32 loopTime;              // used by rewind, layer (debug)

    // menulayer
    Button btns[MENU_BUTTON_COUNT];  // used by layer
    texture_t*  btn_inactive_tex;    // used by layer
    texture_t*  btn_hover_tex;       // used by layer
    texture_t*  btn_pressed_tex;     // used by layer
    texture_t*  greyout_tex;         // used by layer
    b32 g_layer_menu_is_active;      // used by layer, game

    b32 render_imgui;                // used by game
};
////////////////////////////////////////////////////////////////////////////////////////////////////

// we use a unity build, see en.wikipedia.org/wiki/Unity_build
#include "camera.cpp"
#include "physics.cpp"
#include "entity.cpp"
#include "layer.cpp"
#include "input.cpp"
#include "player.cpp"
#include "resourcemgr.cpp"
#include "rewind.cpp"
#include "levelgen.cpp"

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

enum Layers { LAYER_GAME, LAYER_MENU, LAYER_IMGUI, LAYER_COUNT };
// TODO maybe use a bool array for keeping track of in-/active layers, i.e. bool[LAYER_COUNT]

game_state_t* state = nullptr;
platform_api_t platform = {0};
const u32 SCREEN_WIDTH  = 1280;
const u32 SCREEN_HEIGHT =  960;

// mem_arena_t game_arena
extern "C" void game_state_update(game_state_t* game_state, platform_api_t platform_api)
{
    state    = game_state;
    if (!game_state->initialized)
    {
        state = new (game_state) game_state_t();
        //mem_arena(&game_arena,)
        //state->entity_arena
    }
    platform = platform_api;
}

extern "C" b32 game_init(game_state_t* game_state)
{
    state = game_state;
    state->window = platform.window_open("hello game", SCREEN_WIDTH, SCREEN_HEIGHT);

    // init layers
    layer_game_init();
    layer_menu_init();
    layer_imgui_init();

    return true; // TODO error handling
}

extern "C" b32 game_quit()
{
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
    f32 dt = UPDATE_INTERVAL;

    if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
        update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
    }

    while (update_iterations > UPDATE_INTERVAL) {
        update_iterations -= UPDATE_INTERVAL;

        // EVENT HANDLING //////////////////////////////////////////////////////////////////////////
        platform.event_loop(&state->game_input);

        // if (globals.game_input.quit_requested) globals.game_running = false;

        // toggle testmenu TODO hardcoded
        if (input_pressed(state->game_input.keyboard.keys['\e']))
            state->g_layer_menu_is_active = !state->g_layer_menu_is_active;

        if (state->game_input.keyboard.f_key_pressed[1])
            state->render_imgui = !state->render_imgui;

        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_handle_event();
                } break;

                case LAYER_MENU:
                {
                    if (state->g_layer_menu_is_active) {
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

        // UPDATE LOOP /////////////////////////////////////////////////////////////////////////////
        for (int layer = LAYER_COUNT; layer >= 0; layer--)
        {
            // TODO hardcoded, implements 'pause' functionality
            if (state->g_layer_menu_is_active) break;
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_update(dt);
                } break;
            }
        }
    }

    state->cycles_left_over = update_iterations;
    state->last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////////////////////////////
    platform.renderer.push_clear({});
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
    platform.render(state->window); // NOTE workaround to get imgui to render
    if (state->render_imgui)
    {
        layer_imgui_begin();
        for (int layer = 0; layer < LAYER_COUNT; layer++)
        {
            switch (layer)
            {
                case LAYER_GAME:
                {
                    layer_game_imgui_render(dt);
                } break;
            }
        }
        layer_imgui_end();
    }
#endif
    platform.renderer.push_present({});
    platform.render(state->window);
}
