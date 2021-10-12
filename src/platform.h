#pragma once

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;
struct game_input_t;

typedef void texture_t; // SDL_Texture, ...
typedef void surface_t; // SDL_Surface, ...
typedef void font_t;    // ttf, ...

struct game_state_t;
struct game_memory_t
{
    game_state_t* game_state;

    // DEBUG
    // struct debug_table* DebugTable;
    // struct debug_state* DebugState;

    // platform_work_queue *HighPriorityQueue;
    // platform_work_queue *LowPriorityQueue;
    // struct renderer_texture_queue *TextureQueue;

    // b32 ExecutableReloaded;
    // platform_api PlatformAPI;
};

typedef platform_window_t* (*platform_window_open_fn)(const char*, u32, u32);
typedef void               (*platform_window_close_fn)(platform_window_t*);

// file operations
struct file_t
{
    void* handle;
    u64   size;   // size in bytes
    u8*   buffer;
};
typedef file_t (*platform_file_load_fn)(const char* file_name);
typedef b32    (*platform_file_save_fn)(u8* file_name, u8* buffer);
typedef void   (*platform_file_close_fn)(file_t file);

typedef void (*platform_event_loop_fn)(game_input_t*);
typedef u32  (*platform_ticks_fn)();
typedef void (*platform_quit_fn)();

typedef void (*platform_render_sprite_fn)(platform_window_t*, texture_t*, rect_t, v3f, f32, u32);
typedef void (*platform_render_texture_fn)(platform_window_t*, texture_t*, rect_t*, rect_t*);
typedef void (*platform_render_clear_fn)(platform_window_t*);
typedef void (*platform_render_present_fn)(platform_window_t*);

typedef texture_t* (*platform_texture_create_from_surface_fn)(platform_window_t*, surface_t*);
typedef texture_t* (*platform_texture_load_fn)(platform_window_t*, const char*);
typedef i32        (*platform_texture_query_fn)(texture_t*, u32*, i32*, i32*, i32*);
typedef i32        (*platform_texture_set_blend_mode_fn)(texture_t*, u32);
typedef i32        (*platform_texture_set_alpha_mod_fn)(texture_t*, u8);
typedef void       (*platform_surface_destroy_fn)(surface_t*);
typedef void       (*platform_font_init_fn)();
typedef font_t*    (*platform_font_load_fn)(const char*, i32);

typedef surface_t* (*platform_text_render_fn)(font_t*, const char*, color_t, u32);

typedef void (*platform_debug_draw_fn)(platform_window_t*, rect_t, v3f, color_t, u32);
typedef u64  (*platform_debug_performance_counter_fn)();

typedef void (*platform_imgui_init_fn)(platform_window_t*, u32, u32);
typedef void (*platform_imgui_destroy_fn)();
typedef void (*platform_imgui_event_handle_fn)(game_input_t*);
typedef void (*platform_imgui_begin_fn)(platform_window_t*);
typedef void (*platform_imgui_end_fn)();

struct platform_api_t
{
    platform_file_load_fn                   file_load;
    platform_file_save_fn                   file_save;
    platform_file_close_fn                  file_close;
    platform_window_open_fn                 window_open;
    platform_window_close_fn                window_close;
    platform_event_loop_fn                  event_loop;
    platform_ticks_fn                       ticks;
    platform_quit_fn                        quit;
    platform_render_sprite_fn               render_sprite;
    platform_render_texture_fn              render_texture;
    platform_render_clear_fn                render_clear;
    platform_render_present_fn              render_present;
    platform_texture_create_from_surface_fn texture_create_from_surface;
    platform_texture_load_fn                texture_load;
    platform_texture_query_fn               texture_query;
    platform_texture_set_blend_mode_fn      texture_set_blend_mode;
    platform_texture_set_alpha_mod_fn       texture_set_alpha_mod;
    platform_surface_destroy_fn             surface_destroy;
    platform_font_init_fn                   font_init;
    platform_font_load_fn                   font_load;
    platform_text_render_fn                 text_render;
    platform_debug_draw_fn                  debug_draw;
    platform_debug_performance_counter_fn   debug_performance_counter;
    platform_imgui_init_fn                  imgui_init;
    platform_imgui_destroy_fn               imgui_destroy;
    platform_imgui_event_handle_fn          imgui_event_handle;
    platform_imgui_begin_fn                 imgui_begin;
    platform_imgui_end_fn                   imgui_end;
};
