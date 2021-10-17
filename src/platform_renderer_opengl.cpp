#include "platform_renderer.h"
#include "utils.h"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

// unity build
//#include "glew.c"

struct renderer_t
{
    SDL_GLContext gl_context;
};

texture_t renderer_load_texture(platform_window_t* window, const char* filename)
{
    GLuint TextureID = 0;

    // TODO make surface upside down (origin in SDL_image is upper left, origin in opengl is bottom left...)
    // TODO OR use stb_image.h
    SDL_Surface* Surface = IMG_Load(filename);
    int Mode = GL_RGB;
    if (Surface->format->BytesPerPixel == 4) Mode = GL_RGBA;

    // By defining STB_IMAGE_IMPLEMENTATION the preprocessor modifies the header file
    // such that it only contains the relevant definition source code, effectively
    // turning the header file into a .cpp file, and that's about it. Now simply
    // include stb_image.h somewhere in your program and compile.

    // For the following texture sections we're going to use an image of a wooden
    // container. To load an image using stb_image.h we use its stbi_load function:

    //int width, height, nrChannels;
    //unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    //if (!data) error();

    // The function first takes as input the location of an image file. It then
    // expects you to give three ints as its second, third and fourth argument
    // that stb_image.h will fill with the resulting image's width, height and
    // number of color channels. We need the image's width and height for
    // generating textures later on.

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    // set some parameters TODO needed ?
    // set the texture wrapping parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);

    //Any other glTex* stuff here

    // cleanup
    // TODO do we have to unbind the texture here ?
    //DestroySurface(Surface);
    //stbi_image_free(data);

    return TextureID;
}

// stubs
texture_t renderer_create_texture_from_surface(platform_window_t* window, surface_t* surface) { return 0; }
i32       renderer_texture_query(texture_t tex, u32* format, i32* access, i32* w, i32* h) { return -1; }
void renderer_destroy(renderer_t* renderer) {}

// GLEW_OK = 0
#define GLEW_ERROR(x) if(x) printf("Error initializing GLEW! %s\n", glewGetErrorString(x));

void renderer_init(platform_window_t* window)
{
    cmds.buf_offset  = cmds.buf;
    cmds.entry_count = 0;

    /* opengl code here */

    // TODO probably needed somewhere else..
    // NOTE SDL specific stuff should move into platform layer
    SDL_GLContext gl_context = SDL_GL_CreateContext(window->handle);
    SDL_ERROR(gl_context); // Failed to create OpenGL context.

    // init GLEW
    // NOTE we don't need glew for the time being (we're only using
    // immediate-mode, i.e. legacy opengl right now)
    //glewExperimental = GL_TRUE;
    //i32 error        = glewInit();
    //GLEW_ERROR(error);

    SDL_ERROR(!SDL_GL_SetSwapInterval(1)); // Couldn't set VSYNC

    printf("%s\n", glGetString(GL_VERSION));
}

void renderer_cmd_buf_process(platform_window_t* window)
{
    u8* curr_entry = cmds.buf;
    for (s32 entry_nr = 0; entry_nr < cmds.entry_count; ++entry_nr)
    {
        render_entry_header_t* entry_header = (render_entry_header_t*) curr_entry;
        switch (entry_header->type)
        {
            case RENDER_ENTRY_TYPE_TEXTURE:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_texture_t* draw_tex = (render_entry_texture_t*) curr_entry;

                const u32 SCREEN_WIDTH = 1280; // TODO hardcoded
                const u32 SCREEN_HEIGHT = 960; // TODO hardcoded

                // NOTE draw_tex->dst is in pixel coordinates (x,w:0-1280,
                // y,h:0-960), but opengl needs screen coordinates from -1 to 1
                // (origin is in the center of the screen)
                f32 screen_x = (draw_tex->dst.x / (SCREEN_WIDTH/2.f))  - 1.f;
                f32 screen_y = (draw_tex->dst.y / (SCREEN_HEIGHT/2.f)) - 1.f;
                f32 screen_w = (draw_tex->dst.w / (SCREEN_WIDTH/2.f));
                f32 screen_h = (draw_tex->dst.h / (SCREEN_HEIGHT/2.f));

                // TODO this is hardcoded for testing, we should make texture_t
                // an opaque ptr instead that is defined by the renderer and in
                // the case of opengl should at least contain the GLuint tex_id
                // & width & height
                f32 TEXTURE_WIDTH  = -1.f;
                f32 TEXTURE_HEIGHT = -1.f;
                if (draw_tex->tex == 1) {
                    TEXTURE_WIDTH  = 352.0f;
                    TEXTURE_HEIGHT = 224.0f;
                } else if (draw_tex->tex == 2) {
                    TEXTURE_WIDTH  = 272.0f;
                    TEXTURE_HEIGHT = 256.0f;
                }

                // NOTE draw_tex->src is in pixel coordinates
                // (x,w:0-texture_width y,h:0-texture_height with origin in top
                // left corner), but opengl needs texture coordinates from 0 to
                // 1 (origin is bottom left corner)
                // TODO change origin
                f32 tex_x = (draw_tex->src.x / TEXTURE_WIDTH);
                f32 tex_y = (draw_tex->src.y / TEXTURE_HEIGHT);
                f32 tex_w = (draw_tex->src.w / TEXTURE_WIDTH);
                f32 tex_h = (draw_tex->src.h / TEXTURE_HEIGHT);

                // printf("tex: %f ",  tex_x);
                // printf("%f ",       tex_y);
                // printf("%f ",       tex_w);
                // printf("%f\n",      tex_h);

                // TODO opengl code here
                glMatrixMode(GL_TEXTURE);
                glLoadIdentity();
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glScalef(1.0, -1.0, 1.0); // NOTE manually flip the image

                glViewport(0,0,1280,960);

                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBindTexture(GL_TEXTURE_2D, (GLuint) draw_tex->tex);

                // how to sample the texture when its larger or smaller
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                // mipmapping stuff, all turned off
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, GL_NEAREST);

                // wrap/clamp uv coords
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

                glBegin(GL_TRIANGLES); // TODO don't use immediate mode
                    glTexCoord2f(tex_x, tex_y);
                    glVertex2f(screen_x, screen_y);
                    glTexCoord2f(tex_x + tex_w, tex_y);
                    glVertex2f(screen_x + screen_w, screen_y);
                    glTexCoord2f(tex_x + tex_w, tex_y + tex_h);
                    glVertex2f(screen_x + screen_w, screen_y + screen_h);

                    glTexCoord2f(tex_x, tex_y);
                    glVertex2f(screen_x, screen_y);
                    glTexCoord2f(tex_x + tex_w, tex_y + tex_h);
                    glVertex2f(screen_x + screen_w, screen_y + screen_h);
                    glTexCoord2f(tex_x, tex_y + tex_h);
                    glVertex2f(screen_x,  screen_y + screen_h);
                glEnd();

                curr_entry += sizeof(render_entry_texture_t);
            } break;

            case RENDER_ENTRY_TYPE_RECT:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;

                // TODO opengl code her
                // ...

                curr_entry += sizeof(render_entry_rect_t);
            } break;

            case RENDER_ENTRY_TYPE_TEXTURE_MOD:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_texture_mod_t* mod = (render_entry_texture_mod_t*) curr_entry;

                // TODO opengl code her
                // ...

                curr_entry += sizeof(render_entry_texture_mod_t);
            } break;

            case RENDER_ENTRY_TYPE_CLEAR:
            {
                curr_entry += sizeof(render_entry_header_t);

                /* opengl code here */
                glClearColor(0.0f,0.0f,0.0f,1.0f);
                glClear(GL_COLOR_BUFFER_BIT);      // clear color buffer

                curr_entry += sizeof(render_entry_clear_t);
            } break;

            case RENDER_ENTRY_TYPE_PRESENT:
            {
                curr_entry += sizeof(render_entry_header_t);

                /* TODO opengl code her */
                // NOTE maybe this shouldn't be a renderer command after all and
                // instead the platform layer just calls this after processing
                // cmds is done
                SDL_GL_SwapWindow(window->handle);

                curr_entry += sizeof(render_entry_present_t);
            } break;

            default:
            {
                UNREACHABLE("OpenGL render command for entry '%u' not implemented", entry_header->type);
            }
        }
    }

    // reset cmd buffer
    cmds.entry_count = 0;
    cmds.buf_offset  = cmds.buf;

#if 0
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
