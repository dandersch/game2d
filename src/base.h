#pragma once

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
    #error "no supported platform detected"
#endif

// NOTE we use SDL on linux & web builds
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
// TODO define EXPORT for COMPILER_CLANG
#ifdef COMPILER_GCC
    // NOTE GCC exports every symbol to the ELF by default (so no keyword would be needed), unless
    // -fvisibility=hidden is specified, then below keyword is needed for exporting
    #define EXPORT __attribute__((visibility("default")))
#endif
#ifdef COMPILER_MSVC
    #define EXPORT __declspec(dllexport)
#endif
// NOTE apparently you can use both __attribute__() & __declspec() with mingw
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

// the 'static' keyword in C has different meanings depending on the context:
#define internal_fn static // for 'static' functions
#define local       static // for 'static' variables inside a function
#define global_var  static // 'static' variables in global scope can be considered global variables when
                           // compiling only one translation unit (unity build)

// numerical limits
#include <float.h>  // for FLT_MAX/MIN
#define U8_MAX  255
#define U16_MAX 65535
#define U32_MIN 0
#define U32_MAX 4294967295
#define U64_MIN 0
#define U64_MAX 18446744073709551615
#define I32_MIN ((i32) 0x80000000)
#define I32_MAX ((i32) 0x7fffffff)
#define S32_MIN ((s32) 0x80000000)
#define S32_MAX ((s32) 0x7fffffff)
#define F32_MAX FLT_MAX
#define F32_MIN -FLT_MAX


// vector types w/ overloaded operators + - * /
union v2u
{
    struct { u32 x; u32 y; };
    struct { u32 u; i32 v; };
    u32 vec[2];
    inline v2u operator+(const v2u& rhs) { return {this->x + rhs.x, this->y + rhs.y}; }
    inline v2u operator-(const v2u& rhs) { return {this->x - rhs.x, this->y - rhs.y}; }
    inline v2u operator*(const v2u& rhs) { return {this->x * rhs.x, this->y * rhs.y}; }
    inline v2u operator/(const v2u& rhs) { return {this->x / rhs.x, this->y / rhs.y}; }

    inline v2u operator+(const i32& rhs) { return {this->x + rhs, this->y + rhs}; }
    inline v2u operator-(const i32& rhs) { return {this->x - rhs, this->y - rhs}; }
    inline v2u operator*(const i32& rhs) { return {this->x * rhs, this->y * rhs}; }
    inline v2u operator/(const i32& rhs) { return {this->x / rhs, this->y / rhs}; }
};
union v3u
{
    struct { u32 x; u32 y; u32 z; };
    u32 vec[3];
    inline v3u operator+(const v3u& rhs) { return {this->x + rhs.x, this->y + rhs.y, this->z + rhs.z}; }
    inline v3u operator-(const v3u& rhs) { return {this->x - rhs.x, this->y - rhs.y, this->z - rhs.z}; }
    inline v3u operator*(const v3u& rhs) { return {this->x * rhs.x, this->y * rhs.y, this->z * rhs.z}; }
    inline v3u operator/(const v3u& rhs) { return {this->x / rhs.x, this->y / rhs.y, this->z / rhs.z}; }

    inline v3u operator+(const i32& rhs) { return {this->x + rhs, this->y + rhs, this->z + rhs}; }
    inline v3u operator-(const i32& rhs) { return {this->x - rhs, this->y - rhs, this->z - rhs}; }
    inline v3u operator*(const i32& rhs) { return {this->x * rhs, this->y * rhs, this->z * rhs}; }
    inline v3u operator/(const i32& rhs) { return {this->x / rhs, this->y / rhs, this->z / rhs}; }
};
union v2i
{
    struct { i32 x; i32 y; };
    struct { i32 u; i32 v; };
    i32 vec[2];
    inline v2i operator+(const v2i& rhs) { return {this->x + rhs.x, this->y + rhs.y}; }
    inline v2i operator-(const v2i& rhs) { return {this->x - rhs.x, this->y - rhs.y}; }
    inline v2i operator*(const v2i& rhs) { return {this->x * rhs.x, this->y * rhs.y}; }
    inline v2i operator/(const v2i& rhs) { return {this->x / rhs.x, this->y / rhs.y}; }

    inline v2i operator+(const i32& rhs) { return {this->x + rhs, this->y + rhs}; }
    inline v2i operator-(const i32& rhs) { return {this->x - rhs, this->y - rhs}; }
    inline v2i operator*(const i32& rhs) { return {this->x * rhs, this->y * rhs}; }
    inline v2i operator/(const i32& rhs) { return {this->x / rhs, this->y / rhs}; }
};
union v3i
{
    struct { i32 x; i32 y; i32 z; };
    i32 vec[3];
    inline v3i operator+(const v3i& rhs) { return {this->x + rhs.x, this->y + rhs.y, this->z + rhs.z}; }
    inline v3i operator-(const v3i& rhs) { return {this->x - rhs.x, this->y - rhs.y, this->z - rhs.z}; }
    inline v3i operator*(const v3i& rhs) { return {this->x * rhs.x, this->y * rhs.y, this->z * rhs.z}; }
    inline v3i operator/(const v3i& rhs) { return {this->x / rhs.x, this->y / rhs.y, this->z / rhs.z}; }

    inline v3i operator+(const i32& rhs) { return {this->x + rhs, this->y + rhs, this->z + rhs}; }
    inline v3i operator-(const i32& rhs) { return {this->x - rhs, this->y - rhs, this->z - rhs}; }
    inline v3i operator*(const i32& rhs) { return {this->x * rhs, this->y * rhs, this->z * rhs}; }
    inline v3i operator/(const i32& rhs) { return {this->x / rhs, this->y / rhs, this->z / rhs}; }
};
union v2f
{
    struct { f32 x; f32 y; };
    struct { f32 u; f32 v; };
    f32 vec[2];
    inline v2f operator+(const v2f& rhs) { return {this->x + rhs.x, this->y + rhs.y}; }
    inline v2f operator-(const v2f& rhs) { return {this->x - rhs.x, this->y - rhs.y}; }
    inline v2f operator*(const v2f& rhs) { return {this->x * rhs.x, this->y * rhs.y}; }
    inline v2f operator/(const v2f& rhs) { return {this->x / rhs.x, this->y / rhs.y}; }

    inline v2f operator+(const f32& rhs) { return {this->x + rhs, this->y + rhs}; }
    inline v2f operator-(const f32& rhs) { return {this->x - rhs, this->y - rhs}; }
    inline v2f operator*(const f32& rhs) { return {this->x * rhs, this->y * rhs}; }
    inline v2f operator/(const f32& rhs) { return {this->x / rhs, this->y / rhs}; }
};
union v3f
{
    struct { f32 x; f32 y; f32 z; };
    f32 vec[3];
    inline v3f operator+(const v3f& rhs) { return {this->x + rhs.x, this->y + rhs.y, this->z + rhs.z}; }
    inline v3f operator-(const v3f& rhs) { return {this->x - rhs.x, this->y - rhs.y, this->z - rhs.z}; }
    inline v3f operator*(const v3f& rhs) { return {this->x * rhs.x, this->y * rhs.y, this->z * rhs.z}; }
    inline v3f operator/(const v3f& rhs) { return {this->x / rhs.x, this->y / rhs.y, this->z / rhs.z}; }

    inline v3f operator+(const f32& rhs) { return {this->x + rhs, this->y + rhs, this->z + rhs}; }
    inline v3f operator-(const f32& rhs) { return {this->x - rhs, this->y - rhs, this->z - rhs}; }
    inline v3f operator*(const f32& rhs) { return {this->x * rhs, this->y * rhs, this->z * rhs}; }
    inline v3f operator/(const f32& rhs) { return {this->x / rhs, this->y / rhs, this->z / rhs}; }
};

// TODO common operations on vectors
// dot product
// cross product
// length/magnitude

// common math operations
#define LERP(min, max, interpolant) (min + interpolant * (max - min))
// NOTE might cause problems when mixing e.g. floats & integers, maybe use a function instead
#define MAP_VALUE_IN_RANGE1_TO_RANGE2(value, range1_min, range1_max, range2_min, range2_max) \
        ((value - range1_min) / (range1_max - range1_min) * (range2_max - range2_min) + range2_min)

// some other useful types
struct rect_t
{
    i32 left, top;
    i32 w, h;
};
struct rectf_t
{
    f32 left, top;
    f32 w, h;
};
struct color_t // NOTE maybe add a 24bit version
{
    u8 r; u8 g; u8 b; u8 a;
};
struct colorf_t // TODO add some implicit conversion functions
{
    f32 r; f32 g; f32 b; f32 a;
    //operator color_t() const { return {(u8)(255*r),(u8)(255*g),(u8)(255*b),(u8)(255*a)}; }
};


// ASSERTIONS //////////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_ASSERTS
#include <signal.h> // for debug breaking
#include <stdio.h>  // for fprintf
    #define REPORT_ASSERT_(expr, file, line)                                        \
        fprintf(stderr, "Assert failed for '%s' in file '%s' at line '%d'\n", expr, file, line)

    // TODO make platform independent
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

    #define UNREACHABLE(msg, ...) { fprintf(stderr,msg,##__VA_ARGS__); ASSERT(false) }
#else
    #define ASSERT(expr)
    #define UNREACHABLE(msg, ...)
#endif

