#pragma once
// TODO renderer api w/ a pushbuffer/commandbuffer scheme, where render commands
// are put into a list by the game layer and processed in the platform (renderer) layer,
// where the specific renderer api (e.g. opengl) processes and 'implements' them

struct renderer_t; // SDL_Renderer, ...

enum render_entry_type_e
{
    //RENDER_ENTRY_TYPE_RECT,
    //RENDER_ENTRY_TYPE_DRAW_SPRITE,
    RENDER_ENTRY_TYPE_LOAD_TEXTURE, // TODO should this be a renderer cmd?
    RENDER_ENTRY_TYPE_TEXTURE,

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

void renderer_init(platform_window_t* window);
void renderer_destroy(renderer_t* renderer);
void renderer_cmd_buf_process();

void renderer_push_texture(render_entry_texture_t draw_tex);
void renderer_push_rect(render_entry_rect_t rect);
void renderer_push_clear(render_entry_clear_t clear);
void renderer_push_present(render_entry_present_t present);
