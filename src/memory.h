#pragma once

// TODO allocate a chunk of memory once at the start through a call to the
// platform layer & use a memory arena for management

#include "entity.h"       // for struct Entity, Tile, MAX_ENTITIES
struct platform_window_t;
#include "input.h"        // for struct game_input_t
#include "platform.h"     // for struct platform_api_t
#include "camera.h"       // for struct camera

struct Button
{
    const char* label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    rect_t     box;
    // TODO maybe use 1 tex w/ an array of rects
    texture_t* tex[COUNT];
    texture_t* text_texture;
    rect_t     text_box;
    std::function<void(void)> callback;
};

struct game_state_t
{
    b32 initialized;

    Entity ents[MAX_ENTITIES];
    u32    temp_count = 0;
    Tile   tiles[MAX_TILES];
    u32    tile_count = 0;

    // globals
    platform_window_t* window;
    b32 game_running = true;
    f32 dt;
    game_input_t game_input;
    f32 last_frame_time;      // global because we need it in the
    f32 cycles_left_over;     // "main_loop" used for emscripten

    platform_api_t platform;

    Camera cam;
    bool debugDraw         = false;
    Entity* focusedEntity = nullptr;
    rect_t focusArrow = {64,32,16,32}; // TODO hardcoded

    u32 actionState;

    // commandprocessor
    u32 cmdIdx = 0;

    // reset
    bool isRewinding = false;
    f32 loopTime     = 0.f;

    // menulayer:
    // buttons, texture_t*
    std::vector<Button> btns;
    texture_t* btn_inactive_tex;
    texture_t* btn_hover_tex;
    texture_t* btn_pressed_tex;
    texture_t* greyout_tex;
    b32 g_layer_menu_is_active;

    b32 render_imgui = false;

    // resourcemanager
    std::unordered_map<std::string, texture_t*> pool_textures{};
    std::unordered_map<std::string, font_t*> pool_fonts{};
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
