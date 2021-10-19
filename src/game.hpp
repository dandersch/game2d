#pragma once

//----------------------------------------------------------------------------------------------//
// NOTE: this file acts as a precompiled header (extension is .hpp so the compiler sees it as a //
// C++-header) if you change something here, make sure you build the pch again                  //
// ---------------------------------------------------------------------------------------------//

#include <cstring> // for memset
#include <cmath>   // for std::round
#include <new>     // for placement new

#include "json.h"

#ifdef IMGUI
  #include "imgui.h"
  //#include "imgui_sdl.h"
  //#include "imgui_impl_sdl.h"
  //#include "imgui_demo.cpp"
  //#include "imgui.cpp"
  //#include "imgui_widgets.cpp"
  //#include "imgui_tables.cpp"
  //#include "imgui_draw.cpp"
#endif
