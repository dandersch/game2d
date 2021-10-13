#pragma once

// TODO allocate a chunk of memory once at the start through a call to the
// platform layer & use a memory arena for management

#include "entity.h"       // for struct Entity, Tile, MAX_ENTITIES
struct platform_window_t;
#include "input.h"        // for struct game_input_t
#include "platform.h"     // for struct platform_api_t
#include "camera.h"       // for struct camera


typedef void (*button_callback_fn)(game_state_t*);
struct Button
{
    const char* label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    rect_t     box;
    // TODO maybe use 1 tex w/ an array of rects
    texture_t* tex[COUNT];
    texture_t* text_texture;
    rect_t     text_box;
    //std::function<void(game_state_t*)> callback;
    button_callback_fn callback;
};

#define MENU_BUTTON_COUNT 3
struct game_state_t
{
    b32 initialized;           // used by game

    Entity ents[MAX_ENTITIES]; // used by entity, game, layer
    u32    temp_count = 0;     // used by entity
    Tile   tiles[MAX_TILES];   // used by entity, game
    u32    tile_count = 0;     // used by entity

    platform_window_t* window; // used by game, layer, resourcemgr
    b32 game_running = true;   // used by game

    game_input_t game_input;   // used by input, layer
    u32 actionState;           // used by input, rewind, player

    f32 last_frame_time;       // global because we need it in the
    f32 cycles_left_over;      // "main_loop" used for emscripten

    //platform_api_t platform; // unused

    Camera cam;                // used by game, layer
    bool debugDraw;            // used by layer
    Entity* focusedEntity;     // used by layer
    rect_t focusArrow;         // used by game, layer

    // commandprocessor
    u32 cmdIdx = 0;            // used by rewind, layer

    // reset
    bool isRewinding;          // used by rewind, layer
    f32 loopTime;              // used by rewind, layer (debug)

    // menulayer
    Button btns[MENU_BUTTON_COUNT]; // used by layer
    texture_t* btn_inactive_tex;    // used by layer
    texture_t* btn_hover_tex;       // used by layer
    texture_t* btn_pressed_tex;     // used by layer
    texture_t* greyout_tex;         // used by layer
    b32 g_layer_menu_is_active;     // used by layer, game

    b32 render_imgui;               // used by game
};

/*
struct game_state
{
    memory_arena TotalArena;

    memory_arena ModeArena;
    memory_arena AudioArena; // TODO(casey): Move this into the audio system proper!

    memory_arena *FrameArena; // TODO(casey): Cleared once per frame
    temporary_memory FrameArenaTemp;

    controlled_hero ControlledHeroes[MAX_CONTROLLER_COUNT];

    task_with_memory Tasks[4];

    game_assets *Assets;

    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;

    audio_state AudioState;
    playing_sound *Music;

    game_mode GameMode;
    union
    {
        game_mode_title_screen *TitleScreen;
        game_mode_cutscene *CutScene;
        game_mode_world *WorldMode;
    };

    dev_mode DevMode;
    dev_ui DevUI;
    in_game_editor Editor;
};
*/
