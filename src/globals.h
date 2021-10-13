#pragma once

#include "base.h"

// TODO use variables
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 960

struct platform_window_t;

#include "input.h"
struct globals_t
{
    // platform_window_t* window;
    // b32 game_running = true;
    // f32 dt;
    //u32           time;
    //f32           accumulator;

    // game_input_t game_input;

    // global because we need it in the "main_loop" used for emscripten
    // f32 last_frame_time;
    // f32 cycles_left_over;
};
//extern globals_t    globals;
#include "platform.h"
extern platform_api_t platform;
