#pragma once
// TODO renderer api w/ a pushbuffer/commandbuffer scheme, where render commands
// are put into a list by the game layer and processed in the platform (renderer) layer,
// where the specific renderer api (e.g. opengl) processes and 'implements' them

struct renderer_t; // SDL_Renderer, ...
struct texture_t;

struct mem_arena_t;

enum render_entry_type_e
{
    //RENDER_ENTRY_TYPE_RECT,
    //RENDER_ENTRY_TYPE_DRAW_SPRITE,
    RENDER_ENTRY_TYPE_LOAD_TEXTURE, // TODO should this be a renderer cmd?

    RENDER_ENTRY_TYPE_TEXTURE,
    RENDER_ENTRY_TYPE_TEXTURE_MOD,

    RENDER_ENTRY_TYPE_RECT,         // debug

    RENDER_ENTRY_TYPE_CLEAR,
    RENDER_ENTRY_TYPE_PRESENT,

    RENDER_ENTRY_TYPE_COUNT,
};
struct render_entry_header_t { u32 type; }; // put in its own struct for casting

struct render_entry_clear_t   { /* maybe color ? */ };
struct render_entry_present_t { /* placeholder */ };

struct render_entry_texture_t // all data needed for this render command
{
    texture_t* tex;
    rect_t     src;
    rect_t     dst;
};

struct render_entry_rect_t // debug
{
    rect_t  rect;
    color_t color;
};

enum texture_blend_mode_e // mirrors SDL_BlendMode for now
{
    TEXTURE_BLEND_MODE_NONE  = 0,
    TEXTURE_BLEND_MODE_BLEND = 1,
    TEXTURE_BLEND_MODE_ADD   = 2,
    TEXTURE_BLEND_MODE_MOD   = 4,
    TEXTURE_BLEND_MODE_MUL   = 8,
    TEXTURE_BLEND_MODE_NO_CHANGE   = 1000, // ours
};
enum texture_scale_mode_e // mirrors SDL_ScaleMode for now TODO use lookup table
{
    TEXTURE_SCALE_MODE_NEAREST,          // == SDL_ScaleModeNearest & GL_NEAREST
    TEXTURE_SCALE_MODE_LINEAR,           // == SDL_ScaleModeLinear & GL_LINEAR.
    TEXTURE_SCALE_MODE_BEST,             // == SDL_ScaleModeBest (anisotropic filtering)
    TEXTURE_SCALE_MODE_NO_CHANGE = 1000, // ours
};
struct render_entry_texture_mod_t
{
    texture_t*           tex;
    texture_blend_mode_e blend;  // SDL_SetTextureBlendMode
    texture_scale_mode_e scale;  // SDL_SetTextureScaleMode
    color_t              rgba;   // SDL_SetTextureColorMod, SDL_SetTextureAlphaMod
};

// TODO what should happen if buffer is full?
#define MAX_CMD_BUF_SIZE 5000000 // TODO find better max
struct renderer_cmd_buf_t
{
    u8  buf[MAX_CMD_BUF_SIZE];
    //u64 base_addr;
    u8* buf_offset;            // TODO better name
    u32 entry_count;
};

/* called by game layer */
void renderer_push_sprite(texture_t* sprite_tex, rect_t sprite_box, v3f position, f32 scale);
void renderer_push_texture(render_entry_texture_t draw_tex);
void renderer_push_texture_mod(render_entry_texture_mod_t mod);
void renderer_push_rect(render_entry_rect_t rect);
void renderer_push_clear(render_entry_clear_t clear);
void renderer_push_present(render_entry_present_t present);
texture_t* renderer_load_texture(platform_window_t* window, const char* filename);
texture_t* renderer_create_texture_from_surface(platform_window_t* window, surface_t* surface);
i32        renderer_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h);

/* called by platform layer */
void renderer_init(platform_window_t* window, mem_arena_t* platform_mem_arena);
void renderer_destroy(renderer_t* renderer);
void renderer_cmd_buf_process();
