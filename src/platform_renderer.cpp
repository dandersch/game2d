#include "platform_renderer.h"

/* This file has all the code for pushing commands onto the renderer command
 * buffer (or push buffer) and the buffer itself. No rendering backend specific
 * code (i.e. OpenGL, SDL, etc.) should be here*/

#include <float.h>  // for FLT_MAX/MIN
#include <limits.h>

#define PUSH_CMD(type, entry)                                                      \
    ASSERT(cmds->buf_offset < &cmds->buf[MAX_CMD_BUF_SIZE]);                       \
    *((render_entry_header_t*) cmds->buf_offset)  = {type};                        \
    sort_buf[sort_entry_count++].cmd_entry        = cmds->buf_offset;              \
    cmds->buf_offset                             += sizeof(render_entry_header_t); \
    *((decltype(entry)*) cmds->buf_offset)        = entry;                         \
    cmds->buf_offset                             += sizeof(entry);                 \
    cmds->entry_count++;

void renderer_push_sprite(texture_t* sprite_tex, rect_t sprite_box, v3f position, f32 scale)
{
    rect_t dst = {(int) position.x, (int) position.y, (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};
    renderer_push_texture({sprite_tex, sprite_box, dst});
}

void renderer_push_texture(render_entry_texture_t draw_tex)
{
    sort_buf[sort_entry_count].key1 = draw_tex.dst.y;
    sort_buf[sort_entry_count].key2 = draw_tex.dst.y + draw_tex.dst.h;
    PUSH_CMD(RENDER_ENTRY_TYPE_TEXTURE, draw_tex);
}

// NOTE doesn't get called
void renderer_push_texture_mod(render_entry_texture_mod_t mod)
{
    PUSH_CMD(RENDER_ENTRY_TYPE_TEXTURE_MOD, mod);
}

// NOTE doesn't get called for now
void renderer_push_rect(render_entry_rect_t rect)
{
    PUSH_CMD(RENDER_ENTRY_TYPE_RECT, rect);
}

void renderer_push_clear(render_entry_clear_t clear)
{
    sort_buf[sort_entry_count].key1 = INT_MIN;
    sort_buf[sort_entry_count].key2 = INT_MIN;
    PUSH_CMD(RENDER_ENTRY_TYPE_CLEAR, clear);
}

void renderer_push_present(render_entry_present_t present)
{
    sort_buf[sort_entry_count].key1 = INT_MAX;
    sort_buf[sort_entry_count].key2 = INT_MAX;
    PUSH_CMD(RENDER_ENTRY_TYPE_PRESENT, present);
}
