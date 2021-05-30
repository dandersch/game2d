#pragma once

#include "pch.h"

#include <SDL_mixer.h>

namespace Sound {

enum SfxType
{
    SFX_PICKUP,
    SFX_MOVE,
    SFX_ATTACK,
    SFX_COUNT
};

//The music that will be played
extern Mix_Music* music;

//The sound effects that will be used
extern Mix_Chunk* sfx_scratch;
extern Mix_Chunk* sfx_high;
extern Mix_Chunk* sfx_medium;
extern Mix_Chunk* sfx_low;

bool initAndLoadSound();
void toggleMusic();
void playSFX(u32 sfx_type);

} // namespace Sound
