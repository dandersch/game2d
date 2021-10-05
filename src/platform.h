#pragma once

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;

struct sprite_t;
struct Entity;
struct game_input_t;

typedef void texture_t;
typedef void surface_t;
typedef void font_t; // ttf

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

// TODO temp in platform_api to get level loading to work
#include "entity.h"
struct game_state_t;
typedef void* (*resourcemgr_texture_load_fn)(const char*, game_state_t*);
typedef void* (*resourcemgr_font_load_fn)(const char*, game_state_t*, i32);
typedef bool (*copyEntity_fn)(const Entity);
typedef bool (*createTile_fn)(const Tile tile);
typedef void (*initializeFrames_fn)(Entity& e);
typedef void (*initialize_fn)(Entity& ent);
typedef b32 (*platform_level_load_fn)(const std::string&, Entity*, u32,
                                      game_state_t*, resourcemgr_texture_load_fn,
                                      resourcemgr_font_load_fn, copyEntity_fn, createTile_fn,
                                      initializeFrames_fn, initialize_fn);
// TODO platform_api struct w/ function pointers and pass it to the game layer on start up
typedef b32 (*platform_reload_code_fn)();
typedef platform_window_t* (*platform_window_open_fn)(const char*, u32, u32);
typedef void               (*platform_window_close_fn)(platform_window_t*);
typedef void (*platform_event_loop_fn)(game_input_t*);
typedef u32  (*platform_ticks_fn)();
typedef void (*platform_quit_fn)();
typedef void (*platform_render_sprite_fn)(platform_window_t*, const sprite_t&, v3f, f32, u32);
typedef void (*platform_render_texture_fn)(platform_window_t*, texture_t*, rect_t*, rect_t*);
typedef void (*platform_render_clear_fn)(platform_window_t*);
typedef void (*platform_render_present_fn)(platform_window_t*);
typedef void (*platform_render_set_draw_color_fn)(platform_window_t*, u8, u8, u8, u8);
typedef texture_t* (*platform_texture_create_from_surface_fn)(platform_window_t*, surface_t*);
typedef texture_t* (*platform_texture_load_fn)(platform_window_t*, const char*);
typedef i32        (*platform_texture_query_fn)(texture_t*, u32*, i32*, i32*, i32*);
typedef i32        (*platform_texture_set_blend_mode_fn)(texture_t*, u32);
typedef i32        (*platform_texture_set_alpha_mod_fn)(texture_t*, u8);
typedef void       (*platform_surface_destroy_fn)(surface_t*);
typedef void       (*platform_font_init_fn)();
typedef font_t*    (*platform_font_load_fn)(const char*, i32);
typedef surface_t* (*platform_text_render_fn)(font_t*, const char*, color_t, u32);
typedef void (*platform_debug_draw_fn)(platform_window_t*, const Entity&, v3f);
typedef void (*platform_debug_draw_rect_fn)(platform_window_t*, rect_t*);
typedef void (*platform_imgui_init_fn)(platform_window_t*, u32, u32);
typedef void (*platform_imgui_destroy_fn)();
typedef void (*platform_imgui_event_handle_fn)(game_input_t*);
typedef void (*platform_imgui_begin_fn)(platform_window_t*);
typedef void (*platform_imgui_end_fn)();
struct platform_api_t
{
    platform_level_load_fn                  level_load;
    platform_window_open_fn                 window_open;
    platform_window_close_fn                window_close;
    platform_event_loop_fn                  event_loop;
    platform_ticks_fn                       ticks;
    platform_quit_fn                        quit;
    platform_render_sprite_fn               render_sprite;
    platform_render_texture_fn              render_texture;
    platform_render_clear_fn                render_clear;
    platform_render_present_fn              render_present;
    platform_render_set_draw_color_fn       render_set_draw_color;
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
    platform_debug_draw_rect_fn             debug_draw_rect;
    platform_imgui_init_fn                  imgui_init;
    platform_imgui_destroy_fn               imgui_destroy;
    platform_imgui_event_handle_fn          imgui_event_handle;
    platform_imgui_begin_fn                 imgui_begin;
    platform_imgui_end_fn                   imgui_end;
};

// window
//platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height);
//void               platform_window_close(platform_window_t* window);
//
//void platform_event_loop(game_input_t* input);
//u32  platform_ticks();
//void platform_quit();
//// void* platform_file_load(char* file_name);
//// void* platform_file_save(char* file_name, char* buffer);
//
//// rendering
//void platform_render_sprite(platform_window_t* window, const sprite_t& spr,
//                            v3f position, f32 scale = 1.0f, u32 flip_type = 0);
//void platform_render_texture(platform_window_t* window, texture_t* tex, rect_t* src, rect_t* dst);
//void platform_render_clear(platform_window_t* window);
//void platform_render_present(platform_window_t* window);
//void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a);
//
//// textures & surfaces
//texture_t* platform_texture_create_from_surface(platform_window_t* window, surface_t* surface);
//texture_t* platform_texture_load(platform_window_t* window, const char* filename);
//i32        platform_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h);
//i32        platform_texture_set_blend_mode(texture_t* tex, u32 mode);
//i32        platform_texture_set_alpha_mod(texture_t* tex, u8 alpha);
//void       platform_surface_destroy(surface_t* surface);
//
//// font & text
//void       platform_font_init();
//font_t*    platform_font_load(const char* filename, i32 ptsize = 50);
//surface_t* platform_text_render(font_t* font, const char* text, color_t color, u32 wrap_len);
//
//// debug
//void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos);
//void platform_debug_draw_rect(platform_window_t* window, rect_t* dst);
//
//// imgui backend
//void platform_imgui_init(platform_window_t* window, u32 screen_width, u32 screen_height);
//void platform_imgui_destroy();
//void platform_imgui_event_handle(game_input_t* input);
//void platform_imgui_begin(platform_window_t* window);
//void platform_imgui_end();
