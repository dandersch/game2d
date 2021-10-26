#pragma once

#include "player.h" // for struct Command

struct Entity;

// COMMAND /////////////////////////////////////////////////////////////////////////////////////////
void command_record_or_replay(Entity* ent, Command* cmd);
void command_init(Entity& ent);
void command_on_update_end();

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
    void update(f32 dt, Entity& e);
    void record(f32 dt, Entity& e);
    void rewind(f32 dt, Entity& e);

} // namespace Rewind

// RESET ///////////////////////////////////////////////////////////////////////////////////////////
namespace Reset
{
    //extern f32 loopTime;
    //extern bool isRewinding;
    extern const f32 rewindFactor; // can slow down/speed up rewind
    extern const f32 TIME_FOR_LOOP;
    void update(f32 dt);
}
