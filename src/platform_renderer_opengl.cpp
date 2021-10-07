#include "platform_renderer.h"
#if 0

// TODO what happens if buffer is full?
#define MAX_CMD_BUF_SIZE 1024
u8 cmd_buf[MAX_CMD_BUF_SIZE];
u64 base_addr = cmd_buf;

// TODO this should actually be the same across renderer backends, so it shouldn't be here
void* renderer_cmd_buf_push(u8* cmd_buf, u32 render_entry_type)
{
    void* result;
    switch (render_entry_type)
    {
        case RENDER_ENTRY_TYPE_RECT:
        {
            cmd_buf[curr_pos] = {render_entry_type_t};
            result            = cmd_buf[curr_pos];
            curr_pos         += sizeof(render_entry_rect_t);
        }
    }

    return result;
}

void renderer_cmd_buf_process()
{
    u8* currentRenderBufferEntry = bufferToRender.baseAddress;
    for (s32 entryNumber = 0; entryNumber < bufferToRender.entryCount; ++entryNumber)
    {
        RenderEntry_Header* entryHeader = (RenderEntry_Header*) currentRenderBufferEntry;
        switch (entryHeader->type)
        {
            case EntryType_DrawRect:
            {
                RenderEntry_DrawRect rectEntry = *((RenderEntry_DrawRect*) currentRenderBufferEntry);

                // Where you actually perform whatever opengl calls you need.
                glBegin(GL_QUADS);
                glColor3fv(rectEntry.color.r, rectEntry.color.g, rectEntry.color.b);
                glVertex2f(rectEntry.pos.x, rectEntry.pos.y);
                glVertex2f(rectEntry.pos.x + rectEntry.width, rectEntry.pos.y);
                glVertex2f(rectEntry.pos.x + rectEntry.width, rectEntry.pos.y + rectEntry.height);
                glVertex2f(rectEntry.pos.x, rectEntry.pos.y + rectEntry.height);
                glEnd();

                glDrawQuads(/*whatever params*/);

                currentRenderBufferEntry += sizeof(RenderEntry_DrawRect);
            } break;
        }
    }

}

#endif
