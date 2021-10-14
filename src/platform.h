#pragma once

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;
struct game_input_t;

// TODO avoid the ifdef
#ifdef USE_OPENGL
  typedef u32   texture_t; // GLuint
#else
  typedef void* texture_t; // SDL_Texture*
#endif

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

typedef void (*platform_render_fn)(platform_window_t*);

typedef texture_t  (*platform_texture_create_from_surface_fn)(platform_window_t*, surface_t*);
typedef texture_t  (*platform_texture_load_fn)(platform_window_t*, const char*);
typedef i32        (*platform_texture_query_fn)(texture_t , u32*, i32*, i32*, i32*);
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

// RENDERER API ////////////////////////////////////////////////////////////////////////////////
#include "platform_renderer.h" // TODO temp
typedef void (*renderer_push_sprite_fn)(texture_t, rect_t, v3f, f32);
typedef void (*renderer_push_texture_fn)(render_entry_texture_t);
typedef void (*renderer_push_texture_mod_fn)(render_entry_texture_mod_t);
typedef void (*renderer_push_rect_fn)(render_entry_rect_t);
typedef void (*renderer_push_clear_fn)(render_entry_clear_t);
typedef void (*renderer_push_present_fn)(render_entry_present_t);
typedef texture_t  (*renderer_load_texture_fn)(platform_window_t*, const char*);
typedef texture_t  (*renderer_create_texture_from_surface_fn)(platform_window_t*, surface_t*);
typedef i32        (*renderer_texture_query_fn)(texture_t, u32*, i32*, i32*, i32*);
struct renderer_api_t
{
    renderer_push_sprite_fn      push_sprite;
    renderer_push_texture_fn     push_texture;
    renderer_push_texture_mod_fn push_texture_mod;
    renderer_push_rect_fn        push_rect;
    renderer_push_clear_fn       push_clear;
    renderer_push_present_fn     push_present; // TODO should the game layer have access to this?

    renderer_load_texture_fn                load_texture;
    renderer_create_texture_from_surface_fn create_texture_from_surface;
    renderer_texture_query_fn               texture_query;
};

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
    platform_render_fn                      render;
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

    renderer_api_t                          renderer;
};
