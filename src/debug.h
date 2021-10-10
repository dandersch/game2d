#pragma once

//#define TIMED_BLOCK(id) timed_block_t timed_block_##id(debug_cycle_counter_##id);
#define TIMED_BLOCK(id) timed_block_t timed_block_##id(__COUNTER__);

#include "platform.h"
extern platform_api_t platform;

struct timed_block_t
{
    u64 start_cycle_count;
    u32 id;

    timed_block_t(u32 id_init)
    {
        id = id_init;
        //BEGIN_TIMED_BLOCK_(start_cycle_count);
        start_cycle_count = platform.debug_performance_counter();
    }
    ~timed_block_t()
    {
        u64 cycle_delta = platform.debug_performance_counter() - start_cycle_count;
        printf("id: %u took %zu cycles\n", id, cycle_delta);
        //END_TIMED_BLOCK_(start_cycle_count, id);
    }
};
