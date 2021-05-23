#include "reset.h"

#include "input.h"
#include "event.h"
#include "command.h"

void RewindFinished();
void StartRewind();

bool Reset::isRewinding         = false;
const f32 Reset::rewindFactor   = 3.0f; // can slow down/speed up rewind
f32 Reset::loopTime             = 0.f;
const f32 Reset::TIME_FOR_LOOP  = 10.f;

void Reset::update(f32 dt)
{
    // count down when rewinding
    if (isRewinding) loopTime -= rewindFactor * dt;
    else loopTime += 1 * dt;

    if (loopTime <= 0.0f) RewindFinished();
    if (loopTime >= TIME_FOR_LOOP) StartRewind();

    if (Input::actionState & ACTION_RESTART) StartRewind();
}

void RewindFinished()
{
    printf("REWIND FINISHED\n"); // TODO add debug logger
    Event evn(EventType::EVENT_RESTART);
    EventSystem::sendEvent(evn);
    Reset::loopTime    = 0.0f;
    Reset::isRewinding = false;
}

void StartRewind()
{
    printf("REWIND INIT\n");
    Event evn(EventType::EVENT_REWIND);
    evn.args["speed"] = (void*) &Reset::rewindFactor; // TODO
    Reset::loopTime    = Reset::TIME_FOR_LOOP;
    Reset::isRewinding = true;

    CommandProcessor::cmdIdx = 0;
}
