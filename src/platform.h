#pragma once

// opaque platform structs
struct platform_window_t;
struct platform_sound_device_t;
struct game_input_t;
struct game_state_t;

// opaque renderer structs
struct texture_t;
typedef void surface_t; // SDL_Surface, ...
//typedef void font_t;    // ttf, ...

struct file_t
{
    void* handle;
    u64   size;   // size in bytes
    u8*   buffer;
};

struct renderer_api_t;
struct platform_api_t
{
    b32     (*init)(const char*, u32, u32, platform_window_t**, renderer_api_t*);
    file_t  (*file_load)(const char* file_name);              // NOTE: used once
    void    (*file_close)(file_t file);                       // NOTE: used once
    void    (*event_loop)(game_input_t*, platform_window_t*);
    u32     (*ticks)();                                       // TODO there must be a better way
    void    (*quit)(platform_window_t*);
    u64     (*debug_performance_counter)();                   // NOTE: used once
};
