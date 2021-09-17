// #include <stdbool.h> // true = 1

// #define internal static
// #define local    static
// #define global   static // only true with 1 translation unit

// PLATFORM DETECTION /////////////////////////////////////////////////////////////////////////////
// see https://sourceforge.net/p/predef/wiki/Home/ for more pre-defined macros
// that can help detect os/compiler/arch/etc.
// OS detection
#ifdef _WIN32
    #define PLATFORM_WIN32
#endif

#ifdef __gnu_linux__
    #define PLATFORM_LINUX
#endif

#ifdef __EMSCRIPTEN__
    #define PLATFORM_WEB
#endif

#if !defined(PLATFORM_LINUX) && !defined(PLATFORM_WIN32) && !defined(PLATFORM_WEB)
    #error "no platform detected"
#endif

// compiler detection
#ifdef __GNUC__
    #define COMPILER_GCC

    // clang also defines __GNUC__ since it implements the gcc extensions
    #ifdef __clang__
        #define COMPILER_CLANG
        #undef COMPILER_GCC
    #endif

    // mingw also defines __GNUC__
    #ifdef __MINGW32__
        #define COMPILER_MINGW
        #undef COMPILER_GCC
    #endif
#endif

#ifdef _MSC_VER
    #define COMPILER_MSVC
#endif

// architecture detection
#if defined(_M_X64) || defined(__amd64__)
    #define ARCH_X64
#else
    #define ARCH_X86 // TODO untested
#endif

// TYPEDEFS ////////////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

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

// ASSERTIONS //////////////////////////////////////////////////////////////////////////////////////
#include <signal.h> // for debug breaking
#ifdef ENABLE_ASSERTS
    #define REPORT_ASSERT(expr, file, line)                                        \
        printf("Assert failed for '%s' in file '%s' at line '%d'\n", expr, file, line)

    // TODO make platform indepent
    #ifdef COMPILER_MSVC
        #define DEBUG_BREAK() DebugBreak()
    #else
        #ifdef COMPILER_MINGW
            // TODO mingw doesn't have SIGTRAP
            #define DEBUG_BREAK() raise(SIGABRT)
        #else
            #define DEBUG_BREAK() raise(SIGTRAP)
        #endif
    #endif

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

// ERRORS //////////////////////////////////////////////////////////////////////////////////////////
#define SDL_ERROR(x) if (!x) { printf("SDL ERROR: %s\n", SDL_GetError()); }
#define MIX_ERROR(x) if (!x) { printf("MIX ERROR: %s\n", Mix_GetError()); }
