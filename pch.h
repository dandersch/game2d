#pragma once

// (mostly) temporary globals
struct RenderWindow;
static RenderWindow* rw;
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT  960

#include "base.h"

#include <string>
#include <vector>
#include <stdio.h>

#ifdef IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"
#endif

#include "glm/glm.hpp"
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include "SDL_pixels.h"
#include "SDL_render.h"
//#include <SDL2/SDL_image.h>
#include <SDL_image.h>
#include <SDL_video.h>
//#include "Box2D/Box2D.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif