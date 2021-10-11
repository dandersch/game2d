#include "platform_renderer.h"

// TODO what happens if buffer is full?
#define MAX_CMD_BUF_SIZE 16384 // TODO find better max
struct renderer_cmd_buf_t
{
    u8  buf[MAX_CMD_BUF_SIZE];
    //u64 base_addr;
    u8* buf_offset;
};

static renderer_cmd_buf_t cmds = {0};

void renderer_init()
{
    //cmds.base_addr = cmds.buf;
    cmds.buf_offset = cmds.buf;
}

void renderer_push_texture(render_entry_type_draw_texture_t draw_tex)
{
    render_entry_type_draw_texture_t* new_loc = (render_entry_type_draw_texture_t*) &cmds.buf_offset;
    *new_loc = draw_tex;
    cmds.buf_offset += sizeof(draw_tex);
    printf("%p\n", cmds.buf_offset);
}

// TODO this should actually be the same across renderer backends, so it shouldn't be here
void* renderer_cmd_buf_push(u8* cmd_buf, u32 render_entry_type)
{
    /*
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
    */

    return nullptr;
}

void renderer_cmd_buf_process()
{
#if 0
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

    // handmade
    //Assert(OpenGL->DepthPeelCount > 0);
    //u32x MaxRenderTargetIndex = OpenGL->DepthPeelCount - 1;
    //u32 OnPeelIndex = 0;
    //u8 *PeelHeaderRestore = 0;
    u32 CurrentRenderTargetIndex = 0xFFFFFFFF;
    //m4x4 Proj = Identity();
    for(u8 *HeaderAt = Commands->PushBufferBase; HeaderAt < Commands->PushBufferDataAt;)
    {
        render_group_entry_header *Header = (render_group_entry_header *)HeaderAt;
        HeaderAt += sizeof(render_group_entry_header);
        void *Data = (uint8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
            case RenderGroupEntryType_render_entry_full_clear:
            {
                HeaderAt += sizeof(render_entry_full_clear);
                render_entry_full_clear *Entry = (render_entry_full_clear *)Data;

                glClearColor(Entry->ClearColor.r, Entry->ClearColor.g, Entry->ClearColor.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_begin_peels:
            {
                HeaderAt += sizeof(render_entry_begin_peels);
                render_entry_begin_peels *Entry = (render_entry_begin_peels *)Data;

                PeelHeaderRestore = (u8 *)Header;
                OpenGLBindFramebuffer(OpenGL, &OpenGL->DepthPeelBuffer, RenderWidth, RenderHeight);

                glScissor(0, 0, RenderWidth, RenderHeight);
                if(OnPeelIndex == MaxRenderTargetIndex)
                {
                    glClearColor(Entry->ClearColor.r, Entry->ClearColor.g, Entry->ClearColor.b, 1.0f);
                }
                else
                {
                    glClearColor(0, 0, 0, 0);
                }
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_end_peels:
            {
                if(OpenGL->Multisampling)
                {
                    opengl_framebuffer *From = &OpenGL->DepthPeelBuffer;
                    opengl_framebuffer *To = OpenGL->DepthPeelResolveBuffer + OnPeelIndex;
                    GLuint Mask = 0;
                    if(OnPeelIndex == 0)
                    {
                        Mask = OpenGL->SinglePixelAllZeroesTexture;
                    }
                    else
                    {
                        Mask = OpenGL->DepthPeelResolveBuffer[OnPeelIndex - 1].ColorHandle[0];
                    }
#if 1
                    ResolveMultisample(OpenGL, From, To, RenderWidth, RenderHeight, Mask);
#else
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, From->FramebufferHandle);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, To->FramebufferHandle);
                    glViewport(0, 0, RenderWidth, RenderHeight);
                    glBlitFramebuffer(0, 0, RenderWidth, RenderHeight,
                                      0, 0, RenderWidth, RenderHeight,
                                      GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT,
                                      GL_NEAREST);
#endif
                }

                if(OnPeelIndex < MaxRenderTargetIndex)
                {
                    HeaderAt = PeelHeaderRestore;
                    OnPeelIndex++;
                }
                else
                {
                    Assert(OnPeelIndex == MaxRenderTargetIndex);

                    opengl_framebuffer *PeelBuffer = GetDepthPeelReadBuffer(OpenGL, 0);
                    OpenGLBindFramebuffer(OpenGL, PeelBuffer, RenderWidth, RenderHeight);
                    OnPeelIndex = 0;
                    glEnable(GL_BLEND);
                }
            } break;

            case RenderGroupEntryType_render_entry_depth_clear:
            {
                glClear(GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_textured_quads:
            {
                OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VertexBuffer);
                OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBuffer);

                HeaderAt += sizeof(render_entry_textured_quads);
                render_entry_textured_quads *Entry = (render_entry_textured_quads *)Data;

                b32x Peeling = (OnPeelIndex > 0);

                render_setup *Setup = &Entry->Setup;

                rectangle2 ClipRect = Setup->ClipRect;
                s32 ClipMinX = S32BinormalLerp(0, ClipRect.Min.x, RenderWidth);
                s32 ClipMinY = S32BinormalLerp(0, ClipRect.Min.y, RenderHeight);
                s32 ClipMaxX = S32BinormalLerp(0, ClipRect.Max.x, RenderWidth);
                s32 ClipMaxY = S32BinormalLerp(0, ClipRect.Max.y, RenderHeight);
                glScissor(ClipMinX, ClipMinY, ClipMaxX - ClipMinX, ClipMaxY - ClipMinY);

                zbias_program *Prog = &OpenGL->ZBiasNoDepthPeel;
                f32 AlphaThreshold = 0.0f;
                if(Peeling)
                {
                    opengl_framebuffer *PeelBuffer = GetDepthPeelReadBuffer(OpenGL, OnPeelIndex - 1);

                    Prog = &OpenGL->ZBiasDepthPeel;
                    OpenGL->glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, PeelBuffer->DepthHandle);
                    OpenGL->glActiveTexture(GL_TEXTURE0);

                    if(OnPeelIndex == MaxRenderTargetIndex)
                    {
                        AlphaThreshold = 0.9f;
                    }
                }

                OpenGL->glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, OpenGL->DiffuseLightAtlasHandle);
                OpenGL->glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, OpenGL->SpecularLightAtlasHandle);

                OpenGL->glActiveTexture(GL_TEXTURE0);

                UseProgramBegin(OpenGL, Prog, Commands, Setup, AlphaThreshold);

                if(Entry->QuadTextures)
                {
                    // NOTE(casey): This is the multiple-dispatch slow path, for
                    // arbitrary sized textures
                    u32 IndexIndex = Entry->IndexArrayOffset;
                    for(u32 QuadIndex = 0;
                        QuadIndex < Entry->QuadCount;
                        ++QuadIndex)
                    {
                        renderer_texture Texture = Entry->QuadTextures[QuadIndex];
                        GLuint TextureHandle = GetSpecialTextureHandleFor(OpenGL, Texture);
                        glBindTexture(GL_TEXTURE_2D_ARRAY, TextureHandle);
                        OpenGL->glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                                                         (GLvoid *)(IndexIndex*sizeof(u16)),
                                                         Entry->VertexArrayOffset);
                        IndexIndex += (6*QuadIndex);
                    }
                }
                else
                {
                    // NOTE(casey): This is the single-dispatch fast path, for texture arrays
                    glBindTexture(GL_TEXTURE_2D_ARRAY, OpenGL->TextureArray);
                    OpenGL->glDrawElementsBaseVertex(GL_TRIANGLES, 6*Entry->QuadCount, GL_UNSIGNED_SHORT,
                                                     (GLvoid *)(Entry->IndexArrayOffset*sizeof(u16)),
                                                     Entry->VertexArrayOffset);
                }
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

                UseProgramEnd(OpenGL, &Prog->Common);
                if(Peeling)
                {
                    OpenGL->glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    OpenGL->glActiveTexture(GL_TEXTURE0);
                }
            } break;

            InvalidDefaultCase;
        }
    }
#endif

}
