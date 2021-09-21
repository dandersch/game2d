#include "sound.h"

/*

Mix_Music* Sound::music       = NULL;
Mix_Chunk* sfx_table[Sound::SFX_COUNT] = {0};

bool Sound::initAndLoadSound()
{
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        MIX_ERROR(false);

        //Load music
    music = Mix_LoadMUS("res/test.wav");
    MIX_ERROR(music);

    //Load sound effects
    sfx_table[Sound::SFX_PICKUP] = Mix_LoadWAV("res/die.wav");
    MIX_ERROR(sfx_table[Sound::SFX_PICKUP]);

    Mix_AllocateChannels(32);

    return true;
}

void Sound::playSFX(u32 sfx_type)
{
    // NOTE apparently passing a nullptr here doesn't matter...
    // NOTE rest of the program halts when this is called
    Mix_PlayChannel( -1, sfx_table[sfx_type], 0);
}

void Sound::toggleMusic()
{
    if (!Mix_PlayingMusic()) {
        Mix_PlayMusic(music, -1);
    } else if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }
}

*/
