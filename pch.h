#pragma once

#include "base.h"

// temporary globals
class RenderWindow;
extern RenderWindow* rw;
extern u32 g_time;
extern f32 dt;
extern f32 accumulator;
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 960

#include <memory>
#include <unordered_map>
#include <map>
#include <functional>
#include <string>
#include <vector>
#include <stdio.h>

#ifdef IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"
#endif

#include <glm/glm.hpp>
#include <SDL.h>
#include "SDL_hints.h"
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_keyboard.h>
#include <SDL_image.h>
#include <SDL_video.h>
#include "SDL_thread.h"
//#include "Box2D/Box2D.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
