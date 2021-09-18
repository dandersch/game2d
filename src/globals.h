#pragma once

#include "base.h"
#include "platform.h"

//class RenderWindow;
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 960

struct globals_t
{
    //RenderWindow*      rw;
    platform_window_t* window;
    bool          game_running;
    f32           dt;
    //u32           time;
    //f32           accumulator;

    // global because we need it in the "main_loop" used for emscripten
    f32           last_frame_time;
    f32           cycles_left_over;
};
extern globals_t globals;
