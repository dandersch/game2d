#pragma once
#include "pch.h"

void* resourcemgr_texture_load(const char* filename);
void* resourcemgr_font_load(const char* filename, i32 ptsize = 50);
b32   resourcemgr_free(const char* filename);
