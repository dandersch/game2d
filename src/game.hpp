#pragma once

//----------------------------------------------------------------------------------------------//
// NOTE: this file acts as a precompiled header (extension is .hpp so the compiler sees it as a //
// C++-header) if you change something here, make sure you build the pch again                  //
// ---------------------------------------------------------------------------------------------//

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstring>
#include <unordered_map>

// for levelgen
//#include "tmxlite/Map.hpp"
//#include "tmxlite/ObjectGroup.hpp"
//#include "tmxlite/TileLayer.hpp"
//#include "tmxlite/Tileset.hpp"
#include "tmxlite/src/ObjectGroup.cpp"
#include "tmxlite/src/Tileset.cpp"
#include "tmxlite/src/miniz.c"
#include "tmxlite/src/ObjectTypes.cpp"
#include "tmxlite/src/detail/pugiconfig.hpp"
#include "tmxlite/src/detail/pugixml.hpp"
#include "tmxlite/src/detail/pugixml.cpp"
#include "tmxlite/src/Object.cpp"
#include "tmxlite/src/TileLayer.cpp"
#include "tmxlite/src/LayerGroup.cpp"
#include "tmxlite/src/FreeFuncs.cpp"
#include "tmxlite/src/miniz.h"
#include "tmxlite/src/ImageLayer.cpp"
#include "tmxlite/src/Map.cpp"
#include "tmxlite/src/Property.cpp"

#include "json.h"

#ifdef IMGUI
  #include "imgui.h"
  #include "imgui_sdl.h"
  #include "imgui_impl_sdl.h"
#endif
