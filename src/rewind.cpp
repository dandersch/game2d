#include "rewind.h"

#include "entity.h"
#include "input.h"

static const u32 FPS       = 60;  // TODO support variable fps
static const u32 TOLERANCE = 100; // NOTE can be zero apparently
const f32 Reset::rewindFactor   = 3.0f; // can slow down/speed up rewind
const f32 Reset::TIME_FOR_LOOP  = 10.f;
const u32 MAX_CMD_COUNT = 5000;

// TODO add rewind_ctx struct and pass that in & out
// should also contain
//   Command     cmds[MAX_CHAR_COUNT][MAX_CMD_COUNT]
//   PointInTime frames[MAX_ENTITY_COUNT][FRAMECOUUNT]
// look up into cmds with a char_id & into frames with an entity_id (or both with entity_id)
//
// struct replay_ctx
// {
//     Command      cmds[][];
//     PointInTime  frames[][];
//     f32 loop_time;
//     u32 curr_cmd_idx;
//     b32 rewinding;
// }

void command_record_or_replay(Entity* ent, Command* record_cmd, u32* cmd_idx)
{
    ASSERT(*cmd_idx < MAX_CMD_COUNT);

    // record command if given
    if (record_cmd) ent->cmds[*cmd_idx] = *record_cmd;

    Command* cmd = &ent->cmds[*cmd_idx];
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


void Rewind::initializeFrames(Entity& e)
{
    const u32 FRAMECOUNT = Reset::TIME_FOR_LOOP * FPS + TOLERANCE;
    e.frames = new PointInTime[FRAMECOUNT];
    //memset(e.frames, 0, sizeof(PointInTime) * FRAMECOUNT);
}


void Rewind::update(f32 dt, Entity& e, f32 loop_time, b32 is_rewinding)
{
    if (is_rewinding)
        rewind(dt, e, loop_time);
    else
        record(dt, e, loop_time);
}


internal_fn u32 TimeToIndex(f32 dt, f32 loop_time) // translate looptime into index in array
{
    u32 index = 0;
    index = (u32) std::round(loop_time / dt); // TODO fixed delta time
    return index;
}


void Rewind::record(f32 dt, Entity& e, f32 loop_time)
{
    e.frames[TimeToIndex(dt, loop_time)].pos    = e.position;
    e.frames[TimeToIndex(dt, loop_time)].state  = e.state;
    e.frames[TimeToIndex(dt, loop_time)].orient = e.orient;
    e.frames[TimeToIndex(dt, loop_time)].active = e.active;
    //e.frames[TimeToIndex(dt, loop_time)].wasSet = true;
}


void Rewind::rewind(f32 dt, Entity& e, f32 loop_time)
{
    // TODO what is this needed for
    //ASSERT(e.frames[TimeToIndex(dt, loop_time)-1].wasSet);

    e.position = e.frames[TimeToIndex(dt, loop_time)].pos;
    e.state    = e.frames[TimeToIndex(dt, loop_time)].state;
    e.orient   = e.frames[TimeToIndex(dt, loop_time)].orient;
    e.active   = e.frames[TimeToIndex(dt, loop_time)].active;
}


void Reset::update(f32 dt, b32* is_rewinding, f32* loop_time, u32* cmd_idx, const u32 action_state)
{
    // count down when rewinding
    if (*is_rewinding) *loop_time -= rewindFactor * dt;
    else *loop_time += 1 * dt;

    if (*loop_time <= 0.0f)
    {
        printf("REWIND FINISHED\n");
        *loop_time    = 0.0f;
        *is_rewinding = false;
    }

    if (*loop_time >= TIME_FOR_LOOP || action_state & ACTION_RESTART) // TODO manual restart is broken
    {
        printf("REWIND INIT\n");
        *loop_time    = Reset::TIME_FOR_LOOP;
        *is_rewinding = true;
        *cmd_idx      = 0;
    }
}
