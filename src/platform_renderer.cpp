#include "platform_renderer.h"

/* This file has all the code for pushing commands onto the renderer command
 * buffer (or push buffer) and the buffer itself. No rendering backend specific
 * code (i.e. OpenGL, SDL, etc.) should be here*/

// TODO what should happen if buffer is full?
#define MAX_CMD_BUF_SIZE 5000000 // TODO find better max
struct renderer_cmd_buf_t
{
    u8  buf[MAX_CMD_BUF_SIZE];
    //u64 base_addr;
    u8* buf_offset;            // TODO better name
    u32 entry_count;
};

static renderer_cmd_buf_t cmds = {0}; // TODO allocate differently

void renderer_push_sprite(texture_t* sprite_tex, rect_t sprite_box, v3f position, f32 scale)
{
    rect_t dst = {(int) position.x, (int) position.y, (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};
    renderer_push_texture({sprite_tex, sprite_box, dst});
}

// TODO this should be just in the renderer api and not in the opengl specific code,
// maybe put into platform_renderer.cpp together with the buffer
void renderer_push_texture(render_entry_texture_t draw_tex)
{
    ASSERT(cmds.buf_offset < &cmds.buf[MAX_CMD_BUF_SIZE]);

    // TODO could be turned into a macro w/ decltype()
    *((render_entry_header_t*) cmds.buf_offset)  = {RENDER_ENTRY_TYPE_TEXTURE};
    cmds.buf_offset                             += sizeof(render_entry_header_t);
    *((render_entry_texture_t*) cmds.buf_offset) = draw_tex;
    cmds.buf_offset                             += sizeof(draw_tex);
    cmds.entry_count++;
}

void renderer_push_texture_mod(render_entry_texture_mod_t mod)
{
    ASSERT(cmds.buf_offset < &cmds.buf[MAX_CMD_BUF_SIZE]);

    *((render_entry_header_t*) cmds.buf_offset)      = {RENDER_ENTRY_TYPE_TEXTURE_MOD};
    cmds.buf_offset                                 += sizeof(render_entry_header_t);
    *((render_entry_texture_mod_t*) cmds.buf_offset) = mod;
    cmds.buf_offset                                 += sizeof(mod);
    cmds.entry_count++;
}

void renderer_push_rect(render_entry_rect_t rect)
{
    ASSERT(cmds.buf_offset < &cmds.buf[MAX_CMD_BUF_SIZE]);
    *((render_entry_header_t*) cmds.buf_offset)  = {RENDER_ENTRY_TYPE_RECT};
    cmds.buf_offset                             += sizeof(render_entry_header_t);
    *((render_entry_rect_t*) cmds.buf_offset)    = rect;
    cmds.buf_offset                             += sizeof(rect);
    cmds.entry_count++;
}

void renderer_push_clear(render_entry_clear_t clear)
{
    ASSERT(cmds.buf_offset < &cmds.buf[MAX_CMD_BUF_SIZE]);
    *((render_entry_header_t*) cmds.buf_offset)  = {RENDER_ENTRY_TYPE_CLEAR};
    cmds.buf_offset                             += sizeof(render_entry_header_t);
    *((render_entry_clear_t*) cmds.buf_offset)   = clear;
    cmds.buf_offset                             += sizeof(clear);
    cmds.entry_count++;
}

void renderer_push_present(render_entry_present_t present)
{
    ASSERT(cmds.buf_offset < &cmds.buf[MAX_CMD_BUF_SIZE]);
    *((render_entry_header_t*) cmds.buf_offset)  = {RENDER_ENTRY_TYPE_PRESENT};
    cmds.buf_offset                             += sizeof(render_entry_header_t);
    *((render_entry_present_t*) cmds.buf_offset) = present;
    cmds.buf_offset                             += sizeof(present);
    cmds.entry_count++;
}
