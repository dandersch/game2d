#include "reset.h"

#include "input.h"
#include "command.h"

void RewindFinished();
void StartRewind();

//bool Reset::isRewinding         = false;
//f32 Reset::loopTime             = 0.f;
const f32 Reset::rewindFactor   = 3.0f; // can slow down/speed up rewind
const f32 Reset::TIME_FOR_LOOP  = 10.f;

#include "memory.h"
extern game_state_t* state;

void Reset::update(f32 dt)
{
    // count down when rewinding
    if (state->isRewinding) state->loopTime -= rewindFactor * dt;
    else state->loopTime += 1 * dt;

    if (state->loopTime <= 0.0f) RewindFinished();
    if (state->loopTime >= TIME_FOR_LOOP) StartRewind();

    if (state->actionState & ACTION_RESTART) StartRewind();
}

void RewindFinished()
{
    printf("REWIND FINISHED\n"); // TODO add debug logger
    state->loopTime    = 0.0f;
    state->isRewinding = false;
}

void StartRewind()
{
    printf("REWIND INIT\n");
    state->loopTime    = Reset::TIME_FOR_LOOP;
    state->isRewinding = true;

    state->cmdIdx = 0;
}
