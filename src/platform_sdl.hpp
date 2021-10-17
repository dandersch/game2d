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
#include <SDL_ttf.h>
#include <SDL_surface.h>
#include <SDL_pixels.h>
#include <SDL_keyboard.h>
#include <SDL_image.h>
#include <SDL_video.h>
#include <SDL_thread.h>
#include <SDL_blendmode.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL_loadso.h>

#ifdef USE_OPENGL
  //#include "glew.h"
  #include <SDL_opengl.h>
  #include <GL/glu.h>
  #include <GL/gl.h>

  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"
#else
  #include <SDL_render.h>
#endif

// IMGUI include TODO maybe move this somewhere else
#ifdef IMGUI
  #include "imgui.h"
  #include "imgui_sdl.h"
  #include "imgui_impl_sdl.h"
#endif

#if defined(PLATFORM_WEB)
  #include <emscripten.h>
#endif
