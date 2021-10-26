#include "rewind.h"

#include "entity.h"
#include "input.h"

static const u32 FPS       = 60;  // TODO support variable fps
static const u32 TOLERANCE = 100; // NOTE can be zero apparently

const f32 Reset::rewindFactor   = 3.0f; // can slow down/speed up rewind
const f32 Reset::TIME_FOR_LOOP  = 10.f;

const u32 MAX_CMD_COUNT = 5000;

void command_record_or_replay(Entity* ent, Command* record_cmd)
{
    ASSERT(state->cmdIdx < MAX_CMD_COUNT);

    // record command if given
    if (record_cmd) ent->cmds[state->cmdIdx] = *record_cmd;

    Command* cmd = &ent->cmds[state->cmdIdx];
    switch (cmd->type)
    {
        case Command::MOVE:   { player_try_move(cmd->movement, *ent);   } break;
        case Command::PICKUP: { player_try_pickup(cmd->movement, *ent); } break;
        case Command::ATTACK: { player_try_attack(cmd->movement, *ent); } break;
        default: { UNREACHABLE("command not implemented\n"); } break;
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


internal_fn u32 TimeToIndex(f32 dt) // translate looptime into index in array
{
    u32 index = 0;
    index = (u32) std::round(state->loopTime / dt); // TODO fixed delta time
    return index;
}


void Rewind::record(f32 dt, Entity& e)
{
    e.frames[TimeToIndex(dt)].pos    = e.position;
    e.frames[TimeToIndex(dt)].state  = e.state;
    e.frames[TimeToIndex(dt)].orient = e.orient;
    e.frames[TimeToIndex(dt)].active = e.active;
    e.frames[TimeToIndex(dt)].wasSet = true;
}


void Rewind::rewind(f32 dt, Entity& e)
{
    ASSERT(e.frames[TimeToIndex(dt)-1].wasSet);

    e.position = e.frames[TimeToIndex(dt)].pos;
    e.state    = e.frames[TimeToIndex(dt)].state;
    e.orient   = e.frames[TimeToIndex(dt)].orient;
    e.active   = e.frames[TimeToIndex(dt)].active;
}


void Reset::update(f32 dt)
{
    // count down when rewinding
    if (state->isRewinding) state->loopTime -= rewindFactor * dt;
    else state->loopTime += 1 * dt;

    if (state->loopTime <= 0.0f)
    {
        printf("REWIND FINISHED\n");
        state->loopTime    = 0.0f;
        state->isRewinding = false;
    }

    if (state->loopTime >= TIME_FOR_LOOP ||
        state->actionState & ACTION_RESTART) // TODO manual restart is broken
    {
        printf("REWIND INIT\n");
        state->loopTime    = Reset::TIME_FOR_LOOP;
        state->isRewinding = true;

        state->cmdIdx = 0;
    }
}
