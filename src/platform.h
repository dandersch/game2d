#pragma once

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;

#include "pch.h"
struct sprite_t;
struct Entity;

platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height);
// void platform_window_close(platform_window_t* window);

void platform_render(platform_window_t* window, const sprite_t& spr,
                     v3f position, f32 scale = 1.0f, u32 flip_type = 0);
void platform_render_clear(platform_window_t* window);
void platform_render_present(platform_window_t* window);
void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a);

void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos);
void platform_debug_draw_rect(platform_window_t* window, rect_t* dst);

// void* platform_texture_load(platform_window_t* window, const char* filename);
// void* platform_font_load(const char* filename);

void*              platform_file_load(char* file_name);
void*              platform_file_save(char* file_name, char* buffer);
