// #include <stdbool.h> // true = 1

// #define internal        static
// #define local_persist   static
// only true when using 1 translation unit:
// #define global_variable static

#include <stdint.h>
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   b8;
typedef int16_t  b16;
typedef int32_t  b32;
typedef int64_t  b64;

typedef float    f32;
typedef double   f64;

#define SDL_ERROR(x) if (!x) { printf("SDL ERROR: %s\n", SDL_GetError()); }
#define MIX_ERROR(x) if (!x) { printf("MIX ERROR: %s\n", Mix_GetError()); }

// ASSERTIONS //////////////////////////////////////////////////////////////////
#include <signal.h> // for debug breaking
#ifdef ENABLE_ASSERTS
#define REPORT_ASSERT(expr, file, line)                                        \
    printf("Assert failed for '%s' in file '%s' at line '%d'\n", expr, file, line)

// TODO make platform indepent
#define DEBUG_BREAK() raise(SIGTRAP)

#define ASSERT(expr)                                        \
    if (expr) { }                                           \
    else                                                    \
    {                                                       \
        REPORT_ASSERT(#expr, __FILE__, __LINE__);           \
        DEBUG_BREAK();                                      \
    }
#else
#define ASSERT(expr)
#endif
