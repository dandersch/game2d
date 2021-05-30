#pragma once

#include "pch.h"

struct Entity;

bool levelgen_load_level(const std::string& file, Entity* ents, u32 max_ents);
