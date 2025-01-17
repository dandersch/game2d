#pragma once

/* opaque pointers */
struct renderer_t; // SDL_Renderer, OpenGL context, ...
struct texture_t;

/* forward declarations */
struct mem_arena_t;
struct platform_window_t;

enum render_entry_type_e
{
    RENDER_ENTRY_TYPE_NONE,

    RENDER_ENTRY_TYPE_LOAD_TEXTURE,
    RENDER_ENTRY_TYPE_TEXTURE,
    RENDER_ENTRY_TYPE_RECT,

    RENDER_ENTRY_TYPE_TRANSFORM,

    RENDER_ENTRY_TYPE_CLEAR,
    RENDER_ENTRY_TYPE_PRESENT,

    RENDER_ENTRY_TYPE_COUNT,
};
struct render_entry_header_t { u32 type; }; // put in its own struct for casting
enum render_entry_flags_e
{
    RENDER_ENTRY_FLAG_NONE,
    RENDER_ENTRY_FLAG_UI,
    RENDER_ENTRY_FLAG_COUNT,
};
#define RENDER_ENTRY_COMMON_DATA struct { u32 flags = 0; };

struct render_entry_clear_t   { /* maybe color ? */ RENDER_ENTRY_COMMON_DATA };
struct render_entry_present_t { /* placeholder */   RENDER_ENTRY_COMMON_DATA };
struct render_entry_texture_t
{
    texture_t* tex;
    rect_t     src;
    rect_t     dst;
    i32        z_idx = 0;

    RENDER_ENTRY_COMMON_DATA
};
struct render_entry_rect_t
{
    rect_t   rect;
    i32      z_idx = -1;
    colorf_t color = {1.0f, 1.0f, 1.0f, 1.0f};

    RENDER_ENTRY_COMMON_DATA
};

struct m4f { f32 e[4][4]; }; // TODO add proper matrix types
struct render_entry_transform_t
{
    m4f mat;
    b32 ui = false;

    RENDER_ENTRY_COMMON_DATA
};

// TODO what should happen if buffer is full?
#define MAX_CMD_BUF_SIZE 5000000 // TODO find better max
struct renderer_cmd_buf_t
{
    u8  buf[MAX_CMD_BUF_SIZE];
    //u64 base_addr;
    u8* buf_offset;            // TODO better name, since this is not actually the offset
    u32 entry_count;
};

/* called by game layer */
void       renderer_cmd_buf_process(platform_window_t*);
void       renderer_push_texture(render_entry_texture_t draw_tex);
void       renderer_push_rect(render_entry_rect_t rect);
void       renderer_push_transform(render_entry_transform_t mtx);
texture_t* renderer_load_texture(const char* filename);

/* convenience macros for the user */
#define RENDERER_PUSH_SPRITE(sprite_tex, sprite_box, position, scale) do        \
{                                                                               \
    rect_t dst = {(int) position.x, (int) position.y,                           \
                  (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};  \
    renderer.push_texture({sprite_tex, sprite_box, dst});                       \
} while (0);

#define RENDERER_PUSH_CAMERA(mtx) do { renderer.push_transform({mtx, 0}); } while (0);
#define RENDERER_PUSH_UI_MTX(mtx) do { renderer.push_transform({mtx, 1}); } while (0);

// constructs the outline of a rectangle using 4 rectangles, used for debugging
#define RENDERER_PUSH_RECT_OUTLINE(rect,thickness) do                                                                      \
{                                                                                                                          \
       render_entry_rect_t top    = { { rect.rect.left, rect.rect.top, rect.rect.w, thickness}, -1, rect.color };          \
       render_entry_rect_t left   = { { rect.rect.left, rect.rect.top, thickness, rect.rect.h}, -1, rect.color };          \
       render_entry_rect_t right  = { { rect.rect.left + rect.rect.w - thickness, rect.rect.top, thickness, rect.rect.h},  \
                                      -1, rect.color };                                                                    \
       render_entry_rect_t bottom = { { rect.rect.left, rect.rect.top + rect.rect.h - thickness, rect.rect.w, thickness }, \
                                      -1, rect.color };                                                                    \
       renderer.push_rect(top);                                                                                            \
       renderer.push_rect(bottom);                                                                                         \
       renderer.push_rect(left);                                                                                           \
       renderer.push_rect(right);                                                                                          \
} while (0);

/* called by platform layer */
void renderer_init(mem_arena_t* platform_mem_arena);
void renderer_destroy(renderer_t* renderer);

struct renderer_api_t
{
    void        (*render)(platform_window_t*);
    void        (*push_texture)(render_entry_texture_t);
    void        (*push_rect)(render_entry_rect_t);
    void        (*push_transform)(render_entry_transform_t mtx);
    texture_t*  (*load_texture)(const char*);
};
