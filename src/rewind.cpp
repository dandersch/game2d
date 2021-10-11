#include "rewind.h"

#include "entity.h"
#include "input.h"

static const u32 FPS       = 60;  // TODO support variable fps
static const u32 TOLERANCE = 100;

#include "memory.h"
extern game_state_t* state;

// COMMAND /////////////////////////////////////////////////////////////////////////////////////////
#define MAX_CMD_COUNT 10000

// TODO use move semantics?
void command_record(Entity& ent, Command cmd)
{
    ASSERT(state->cmdIdx <= MAX_CMD_COUNT - 1);

    ent.cmds[state->cmdIdx] = cmd;
    command_exec(ent, ent.cmds[state->cmdIdx]);
}

void command_replay(Entity& ent)
{
    ASSERT(state->cmdIdx <= MAX_CMD_COUNT - 1);

    command_exec(ent, ent.cmds[state->cmdIdx]);
}

// TODO use move semantics?
void command_exec(Entity& ent, Command cmd)
{
    switch (cmd.type)
    {
    case Command::MOVE:
        player_try_move(cmd.movement, ent);
        break;

    case Command::PICKUP:
        player_try_pickup(cmd.movement, ent);
        break;

    case Command::ATTACK:
        player_try_attack(cmd.movement, ent);
        break;

    default:
        break;
    }
}

void command_init(Entity& ent)
{
    ent.cmds = new Command[MAX_CMD_COUNT];
}

void command_on_update_end()
{
    state->cmdIdx++;
}

void Rewind::initializeFrames(Entity& e)
{
    const u32 FRAMECOUNT = Reset::TIME_FOR_LOOP * FPS + TOLERANCE;
    e.frames = new PointInTime[FRAMECOUNT];
    //memset(e.frames, 0, sizeof(PointInTime) * FRAMECOUNT);
}

void Rewind::update(f32 dt, Entity& e)
{
    if (state->isRewinding)
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
    index = (u32) std::round(state->loopTime / dt); // TODO fixed delta time
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

// RESET ///////////////////////////////////////////////////////////////////////////////////////////
void RewindFinished();
void StartRewind();

//bool Reset::isRewinding         = false;
//f32 Reset::loopTime             = 0.f;
const f32 Reset::rewindFactor   = 3.0f; // can slow down/speed up rewind
const f32 Reset::TIME_FOR_LOOP  = 10.f;

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
