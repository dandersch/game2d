#pragma once
#if 0
// TODO renderer api w/ a pushbuffer/commandbuffer scheme, where render commands
// are put into a list by the game layer and processed in the platform (renderer) layer,
// where the specific renderer api (e.g. opengl) processes and 'implements' them

enum render_entry_type
{
    RENDER_ENTRY_TYPE_RECT,
    RENDER_ENTRY_TYPE_DRAW_SPRITE,
    RENDER_ENTRY_TYPE_LOAD_TEXTURE, // TODO should this be a renderer cmd?
    RENDER_ENTRY_TYPE_CLEAR,

    RENDER_ENTRY_TYPE_COUNT,
};

// put in its own struct for casting
struct render_entry_header
{
    u32 type;
};

struct render_entry_type_rect
{
    // all data needed for this render command
    rect_t  rect;
    color_t color;
};

// to issue the render command, we push it on the buffer with the right header
//render_entry_rect_t render_entry_rect = renderer_cmd_buf_push(cmd_buf, RENDER_ENTRY_TYPE_RECT);
void* renderer_cmd_buf_push(u8* cmd_buf, u32 render_entry_type);
// renderer_push_sprite();
#endif