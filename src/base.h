#pragma once

// STL includes TODO get rid of these
#include <string>

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

// NOTE we use SDL on linux & web builds
// TODO PLATFORM_WEB does not seem to get defined or not early enough
#if defined(PLATFORM_LINUX) || defined(PLATFORM_WEB)
    #define PLATFORM_SDL
#endif

// compiler detection
#ifdef __GNUC__
    #define COMPILER_GCC

    // NOTE clang also defines __GNUC__ since it implements the gcc extensions
    #ifdef __clang__
        #define COMPILER_CLANG
        #undef COMPILER_GCC
    #endif

    // NOTE mingw also defines __GNUC__
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

// export declarations for .dll/.so
#ifdef COMPILER_GCC
    // NOTE GCC exports every symbol to the ELF by default (so no keyword would be needed), unless
    // -fvisibility=hidden is specified, then below keyword is needed for exporting
    #define EXPORT __attribute__((visibility("default")))
#endif
#ifdef COMPILER_MSVC
    #define EXPORT __declspec(dllexport)
#endif
// NOTE apparently you can use both __attribute__() & __declspec() on mingw
#ifdef COMPILER_MINGW
    #define EXPORT __declspec(dllexport)
#endif

// define c calling convention macro
#ifdef COMPILER_MSVC
    #define CDECL  __cdecl
#endif
#ifdef COMPILER_GCC
    //#define CDECL __attribute__((__cdecl__)) // NOTE this define can cause warnings
    #define CDECL
#endif
#ifdef COMPILER_MINGW
    // NOTE mingw doesn't seem to care if this is defined or not
    #define CDECL  __cdecl
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

// TODO "override" some keywords
// #define internal static
#define local    static
// #define global   static // NOTE only true with 1 translation unit

// TODO numeric limits, i.e. U32_MAX, I32_MIN, F32_MAX etc.

// vector types
// TODO overload operators +-*/
union v2u
{
    struct { u32 x; u32 y; };
    u32 v[2];
};
union v3u
{
    struct { u32 x; u32 y; u32 z; };
    u32 v[3];
};
union v2i
{
    struct { i32 x; i32 y; };
    i32 v[2];
};
union v3i
{
    struct { i32 x; i32 y; i32 z; };
    i32 v[3];
};
union v2f
{
    struct { f32 x; f32 y; };
    f32 v[2];
};
union v3f
{
    struct { f32 x; f32 y; f32 z; };
    f32 v[3];
};

// TODO common operations on vectors

// some other useful types
struct rect_t
{
    i32 x, y;
    i32 w, h;
};
struct point_t
{
    i32 x;
    i32 y;
};
struct color_t // NOTE maybe add a 24bit version
{
    u8 r; u8 g; u8 b; u8 a;
};


// ASSERTIONS //////////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_ASSERTS
#include <signal.h> // for debug breaking
    #define REPORT_ASSERT_(expr, file, line)                                        \
        printf("Assert failed for '%s' in file '%s' at line '%d'\n", expr, file, line)

    // TODO make platform indepent
    #ifdef COMPILER_MSVC
        #define DEBUG_BREAK() DebugBreak()
    #else
        #ifdef COMPILER_MINGW
            #define DEBUG_BREAK() raise(SIGABRT) // NOTE mingw doesn't define SIGTRAP (?)
        #else
            #define DEBUG_BREAK() raise(SIGTRAP)
        #endif
    #endif

    #define ASSERT(expr)                                        \
        if (expr) { }                                           \
        else                                                    \
        {                                                       \
            REPORT_ASSERT_(#expr, __FILE__, __LINE__);          \
            DEBUG_BREAK();                                      \
        }

    // TODO make UNREACHABLE work like printf, i.e. allow format strings
    #define UNREACHABLE(msg) { printf(msg); ASSERT(false) }
#else
    #define ASSERT(expr)
    #define UNREACHABLE
#endif

