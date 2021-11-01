#pragma once

#include "player.h" // for struct Command

struct Entity;

// COMMAND /////////////////////////////////////////////////////////////////////////////////////////
void command_record_or_replay(Entity* ent, Command* record_cmd, u32* cmd_idx);
void command_init(Entity& ent);

// REWIND //////////////////////////////////////////////////////////////////////////////////////////
struct PointInTime
{
    v3f pos    = {0,0,0}; // 12b
    u32 state  = 0;       //  4b
    u32 orient = 0;       //  4b
    b32 active = true;    //  4b
    b8  wasSet = false;   // NOTE for debugging
    // TODO use flags bitfield for animstate & orientation & dead
    //public Dictionary<string, object> custom; // for custom properties
};

namespace Rewind
{
    void initializeFrames(Entity& e);
    void update(f32 dt, Entity& e, f32 loop_time, b32 is_rewinding);
    void record(f32 dt, Entity& e, f32 loop_time);
    void rewind(f32 dt, Entity& e, f32 loop_time);

} // namespace Rewind

// RESET ///////////////////////////////////////////////////////////////////////////////////////////
namespace Reset
{
    //extern f32 loopTime;
    //extern bool isRewinding;
    extern const f32 rewindFactor; // can slow down/speed up rewind
    extern const f32 TIME_FOR_LOOP;
    void update(f32 dt, b32* is_rewinding, f32* loop_time, u32* cmd_idx, const u32 action_state);
}
