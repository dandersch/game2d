#include "rewind.h"

#include "entity.h"
#include "reset.h"
#include <cmath>

static const u32 FPS       = 60;  // TODO support variable fps
static const u32 TOLERANCE = 100;

void Rewind::initializeFrames(Entity& e)
{
    const u32 FRAMECOUNT = Reset::TIME_FOR_LOOP * FPS + TOLERANCE;
    e.frames = new PointInTime[FRAMECOUNT];
    //memset(e.frames, 0, sizeof(PointInTime) * FRAMECOUNT);
}

void Rewind::update(f32 dt, Entity& e)
{
    if (Reset::isRewinding)
        rewind(dt, e);
    else
        record(dt, e);
}

void Rewind::record(f32 dt, Entity& e)
{
    e.frames[TimeToIndex(dt)].pos    = e.position;
    e.frames[TimeToIndex(dt)].state  = e.state;
    e.frames[TimeToIndex(dt)].orient = e.orient;
    e.frames[TimeToIndex(dt)].active = e.active;
    e.frames[TimeToIndex(dt)].wasSet = true;
}

u32 Rewind::TimeToIndex(f32 dt)
{
    u32 index = 0;
    // translate looptime into index in array
    index = (u32) std::round(Reset::loopTime / dt); // TODO fixed delta time
    return index;
}


void Rewind::rewind(f32 dt, Entity& e)
{
    ASSERT(e.frames[TimeToIndex(dt)-1].wasSet);

    e.position = e.frames[TimeToIndex(dt)].pos     ;
    e.state    = e.frames[TimeToIndex(dt)].state   ;
    e.orient   = e.frames[TimeToIndex(dt)].orient  ;
    e.active   = e.frames[TimeToIndex(dt)].active  ;
}
