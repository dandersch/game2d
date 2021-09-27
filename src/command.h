#pragma once

#include "player.h"

struct Entity;

namespace CommandProcessor
{

// TODO use move semantics?
void record(Entity& ent, Command cmd);
void replay(Entity& ent);
void initialize(Entity& ent);
void execute(Entity& ent, Command cmd);
void onEndUpdate();

//extern u32 cmdIdx;

} // namespace CommandProcessor
