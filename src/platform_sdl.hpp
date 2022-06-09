#pragma once

//----------------------------------------------------------------------------------------------//
// NOTE: this file acts as a precompiled header (extension is .hpp so the compiler sees it as a //
// C++-header) if you change something here, make sure you build the pch again                  //
// ---------------------------------------------------------------------------------------------//

#include "base.h" // needs to be at top for #defines

#include <SDL.h>
#include <SDL_hints.h>
#include <SDL_rect.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_timer.h>
#include <SDL_surface.h>
#include <SDL_pixels.h>
#include <SDL_keyboard.h>
#include <SDL_video.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL_loadso.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#ifdef USE_OPENGL
  #define GLEW_STATIC
  #include "ext/glew.h"
  #include <SDL_opengl.h>
  #include <GL/glu.h>
  #include <GL/gl.h>
  #include "ext/glew.c"

#else
  #include <SDL_render.h>
#endif

#if defined(PLATFORM_WEB)
  #include <emscripten.h>
#endif
