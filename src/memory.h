#pragma once

// NOTE testing memory arena ///////////////////////////////////////////////////////////////////
struct mem_arena_t
{
    void* base_addr;
    void* curr_addr;
    u64   total_size;
};
void mem_arena_init(mem_arena_t* mem_arena, u64 size_in_bytes)
{
    mem_arena->base_addr  = malloc(size_in_bytes);
    memset(mem_arena->base_addr, 0, size_in_bytes); // zero memory out by default
    mem_arena->total_size = size_in_bytes;
    mem_arena->curr_addr  = mem_arena->base_addr;
}
void* mem_arena_alloc(mem_arena_t* mem_arena, u64 size_in_bytes)
{
    void* allocation = mem_arena->curr_addr;
    mem_arena->curr_addr = (u8*) mem_arena->curr_addr + size_in_bytes;
    ASSERT(mem_arena->curr_addr <= ((u8*) mem_arena->base_addr + mem_arena->total_size));
    return allocation;
}
void mem_arena_nested_init(mem_arena_t* parent_arena, mem_arena_t* child_arena, u64 size_in_bytes)
{
    child_arena->total_size = size_in_bytes;
    child_arena->base_addr  = mem_arena_alloc(parent_arena, size_in_bytes);
    child_arena->curr_addr  = child_arena->base_addr;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
struct game_memory
{
        game_state *GameState;
        b32 ExecutableReloaded;
        platform_api PlatformAPI;
}

struct game_state
{
    memory_arena TotalArena;

    memory_arena ModeArena;
    memory_arena AudioArena; // TODO(casey): Move this into the audio system proper!

    memory_arena *FrameArena; // TODO(casey): Cleared once per frame
    temporary_memory FrameArenaTemp;

    controlled_hero ControlledHeroes[MAX_CONTROLLER_COUNT];

    task_with_memory Tasks[4];

    game_assets *Assets;

    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;

    audio_state AudioState;
    playing_sound *Music;

    game_mode GameMode;
    union
    {
        game_mode_title_screen *TitleScreen;
        game_mode_cutscene *CutScene;
        game_mode_world *WorldMode;
    };

    dev_mode DevMode;
    dev_ui DevUI;
    in_game_editor Editor;
};
*/
