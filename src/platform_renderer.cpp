#include "platform_renderer.h"

/* This file has all the code for pushing commands onto the renderer command
 * buffer (or push buffer) and the buffer itself. No rendering backend specific
 * code (i.e. OpenGL, SDL, etc.) should be here */

renderer_cmd_buf_t* cmds;

#define PUSH_CMD(type, entry)                                                      \
    ASSERT(cmds->buf_offset < &cmds->buf[MAX_CMD_BUF_SIZE]);                       \
    *((render_entry_header_t*) cmds->buf_offset)  = {type};                        \
    ASSERT(sort_entry_count < MAX_SORT_BUF_SIZE);                                  \
    sort_buf[sort_entry_count++].cmd_entry        = cmds->buf_offset;              \
    cmds->buf_offset                             += sizeof(render_entry_header_t); \
    *((decltype(entry)*) cmds->buf_offset)        = entry;                         \
    cmds->buf_offset                             += sizeof(entry);                 \
    cmds->entry_count++;

// SORTING
struct sort_entry
{
    i32   key1;      // upper y coord
    i32   key2;      // lower y coord
    i32   z_key;
    void* cmd_entry;
};
#define MAX_SORT_BUF_SIZE 20000 // TODO find better max, right now about 10500 cmds per frame
sort_entry sort_buf[MAX_SORT_BUF_SIZE] = {};
u32 sort_entry_count                   = 0;

internal_fn void renderer_sort_buffer()
{
    // TODO right now there is an issue with the y-sorting at the edge of a tile
    // seems to be connected with the "1 in front of 2" case
    // we could workaround this by using z_index...
    qsort(sort_buf, sort_entry_count, sizeof(sort_buf[0]),
          [](const void* elem1, const void* elem2)
          {
              i32 a = ((sort_entry*) elem1)->key1;
              i32 b = ((sort_entry*) elem1)->key2;
              i32 c = ((sort_entry*) elem2)->key1;
              i32 d = ((sort_entry*) elem2)->key2;

              i32 z1 = ((sort_entry*) elem1)->z_key;
              i32 z2 = ((sort_entry*) elem2)->z_key;
              if (z1 > z2) { return -1; }
              if (z1 < z2) { return  1; }

              // TODO could be simplified perhaps
              if (a < c && a < d && b < c && b < d) { return -1; } // 1 is above 2
              if (a > c && a > d && b > c && b > d) { return  1; } // 1 is under 2
              if (a < c && a < d && b > c && b < d) { return -1; } // 1 is behind 2
              if (a > c && a < d && b > c && b > d) { return  1; } // 1 is in front of 2
              if (a > c && a < d && b > c && b < d) { return -1; } // 1 is between 2 NOTE doesn't seem to do anything
              return 0;
          });
}


void renderer_push_sprite(texture_t* sprite_tex, rect_t sprite_box, v3f position, f32 scale)
{
    rect_t dst = {(int) position.x, (int) position.y, (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};
    renderer_push_texture({sprite_tex, sprite_box, dst});
}


void renderer_push_texture(render_entry_texture_t draw_tex)
{
    sort_buf[sort_entry_count].key1  = draw_tex.dst.top;
    sort_buf[sort_entry_count].key2  = draw_tex.dst.top + draw_tex.dst.h;
    sort_buf[sort_entry_count].z_key = draw_tex.z_idx;
    PUSH_CMD(RENDER_ENTRY_TYPE_TEXTURE, draw_tex);
}


void renderer_push_rect(render_entry_rect_t rect)
{
    sort_buf[sort_entry_count].key1  = rect.rect.top;
    sort_buf[sort_entry_count].key2  = rect.rect.top + rect.rect.h;
    sort_buf[sort_entry_count].z_key = rect.z_idx;
    PUSH_CMD(RENDER_ENTRY_TYPE_RECT, rect);
}


// constructs the outline of a rectangle using 4 rectangles, used for debugging
void renderer_push_rect_outline(render_entry_rect_t rect, i32 thickness)
{
    render_entry_rect_t top    = { { rect.rect.left, rect.rect.top, rect.rect.w, thickness}, -1, rect.color };
    render_entry_rect_t left   = { { rect.rect.left, rect.rect.top, thickness, rect.rect.h}, -1, rect.color };
    render_entry_rect_t right  = { { rect.rect.left + rect.rect.w - thickness, rect.rect.top, thickness, rect.rect.h},
                                   -1, rect.color };
    render_entry_rect_t bottom = { { rect.rect.left, rect.rect.top + rect.rect.h - thickness, rect.rect.w, thickness },
                                   -1, rect.color };
    renderer_push_rect(top);
    renderer_push_rect(bottom);
    renderer_push_rect(left);
    renderer_push_rect(right);
}


// NOTE doesn't get called
void renderer_push_texture_mod(render_entry_texture_mod_t mod)
{
    PUSH_CMD(RENDER_ENTRY_TYPE_TEXTURE_MOD, mod);
}



void renderer_push_clear(render_entry_clear_t clear)
{
    sort_buf[sort_entry_count].key1  = I32_MIN;
    sort_buf[sort_entry_count].key2  = I32_MIN;
    sort_buf[sort_entry_count].z_key = I32_MAX;
    PUSH_CMD(RENDER_ENTRY_TYPE_CLEAR, clear);
}


void renderer_push_present(render_entry_present_t present)
{
    sort_buf[sort_entry_count].key1  = I32_MAX;
    sort_buf[sort_entry_count].key2  = I32_MAX;
    sort_buf[sort_entry_count].z_key = I32_MIN;
    PUSH_CMD(RENDER_ENTRY_TYPE_PRESENT, present);
}
