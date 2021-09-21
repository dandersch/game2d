#pragma once
#include "pch.h"

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;

struct sprite_t;
struct Entity;
struct game_input_t;

typedef void texture_t;
typedef void surface_t;
typedef void font_t; // ttf

// window
platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height);
void               platform_window_close(platform_window_t* window);

void platform_event_loop(game_input_t* input);
u32  platform_ticks();
void platform_quit();
// void* platform_file_load(char* file_name);
// void* platform_file_save(char* file_name, char* buffer);

// rendering
void platform_render_sprite(platform_window_t* window, const sprite_t& spr,
                            v3f position, f32 scale = 1.0f, u32 flip_type = 0);
void platform_render_texture(platform_window_t* window, texture_t* tex, rect_t* src, rect_t* dst);
void platform_render_clear(platform_window_t* window);
void platform_render_present(platform_window_t* window);
void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a);

// textures & surfaces
texture_t* platform_texture_create_from_surface(platform_window_t* window, surface_t* surface);
texture_t* platform_texture_load(platform_window_t* window, const char* filename);
i32        platform_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h);
i32        platform_texture_set_blend_mode(texture_t* tex, u32 mode);
i32        platform_texture_set_alpha_mod(texture_t* tex, u8 alpha);
void       platform_surface_destroy(surface_t* surface);

// font & text
void       platform_font_init();
font_t*    platform_font_load(const char* filename, i32 ptsize = 50);
surface_t* platform_text_render(font_t* font, const char* text, color_t color, u32 wrap_len);

// debug
void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos);
void platform_debug_draw_rect(platform_window_t* window, rect_t* dst);

// imgui backend
void platform_imgui_init(platform_window_t* window, u32 screen_width, u32 screen_height);
void platform_imgui_destroy();
void platform_imgui_event_handle(game_input_t* input);
void platform_imgui_begin(platform_window_t* window);
void platform_imgui_end();
