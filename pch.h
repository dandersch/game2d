#pragma once

#include "base.h"

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
//#include "Box2D/Box2D.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
