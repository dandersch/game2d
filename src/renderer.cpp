#include "renderer.h"

/* This file has all the code for pushing commands onto the renderer command
 * buffer (or push buffer) and the buffer itself. No rendering backend specific
 * code (i.e. OpenGL, SDL, etc.) should be here */

renderer_cmd_buf_t* cmds;

#define PUSH_CMD(type, entry) do {                                                 \
    ASSERT(cmds->buf_offset < &cmds->buf[MAX_CMD_BUF_SIZE]);                       \
    *((render_entry_header_t*) cmds->buf_offset)  = {type};                        \
    ASSERT(sort_entry_count < MAX_SORT_BUF_SIZE);                                  \
    sort_buf[sort_entry_count++].cmd_entry        = cmds->buf_offset;              \
    cmds->buf_offset                             += sizeof(render_entry_header_t); \
    *((decltype(entry)*) cmds->buf_offset)        = entry;                         \
    cmds->buf_offset                             += sizeof(entry);                 \
    cmds->entry_count++; } while (0)

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


void renderer_push_transform(render_entry_transform_t mtx)
{
    sort_buf[sort_entry_count].key1  = I32_MIN;
    sort_buf[sort_entry_count].key2  = I32_MIN;
    sort_buf[sort_entry_count].z_key = I32_MAX;
    PUSH_CMD(RENDER_ENTRY_TYPE_TRANSFORM, mtx);
}


renderer_api_t renderer_api =
{
  &renderer_cmd_buf_process,
    &renderer_push_texture,
    &renderer_push_rect,
    &renderer_push_transform,
    &renderer_load_texture,
};


internal_fn void renderer_push_clear(render_entry_clear_t clear)
{
    sort_buf[sort_entry_count].key1  = I32_MIN;
    sort_buf[sort_entry_count].key2  = I32_MIN;
    sort_buf[sort_entry_count].z_key = I32_MAX;
    PUSH_CMD(RENDER_ENTRY_TYPE_CLEAR, clear);
}


internal_fn void renderer_push_present(render_entry_present_t present)
{
    sort_buf[sort_entry_count].key1  = I32_MAX;
    sort_buf[sort_entry_count].key2  = I32_MAX;
    sort_buf[sort_entry_count].z_key = I32_MIN;
    PUSH_CMD(RENDER_ENTRY_TYPE_PRESENT, present);
}

#ifdef USE_OPENGL // NOTE we could compile the renderer as a dll in the future...
  #include "renderer_opengl.cpp"
#else
  #include "renderer_sdl.cpp"
#endif
