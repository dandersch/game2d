#include "platform_renderer.h"
#include "utils.h"

// unity build
//#include "glew.c" // put in pch?

struct renderer_t
{
    SDL_GLContext gl_context;
};

struct texture_t
{
    u32 id;
    u32 width;
    u32 height;
    // ...
};

const char* vertex_shader_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 in_pos;\n"
    "layout (location = 1) in vec2 in_tex_coords;\n"
    "out vec2 out_tex_coords;\n" // TODO
    "void main()\n"
    "{\n"
        "gl_Position = vec4(in_pos.x, in_pos.y, 1.0, 1.0);\n"
        "out_tex_coords = in_tex_coords;\n" // TODO
    "}\0";

const char* fragment_shader_src =
    "#version 330 core\n"
    "in vec2 in_tex_coords;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D u_texture;\n" // TODO
    "void main()\n"
    "{\n"
        "FragColor = texture(u_texture, in_tex_coords);\n"
    "}\0";

global u32 prog_id;

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
    // NOTE we don't need glew for the time being (we're only using immediate-mode, i.e. legacy opengl right now)
    //glewExperimental = GL_TRUE;
    //i32 error        = glewInit();
    //GLEW_ERROR(error);

    SDL_ERROR(!SDL_GL_SetSwapInterval(1)); // Couldn't set VSYNC

    printf("%s\n", glGetString(GL_VERSION));

    //i32 success;
    //u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(vert_shader, 1, &vertex_shader_src, NULL);
    //glCompileShader(vert_shader);
    //glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    //if (!success) ASSERT(false);

    //u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    //glShaderSource(frag_shader, 1, fragment_shader_src, NULL);
    //glCompileShader(frag_shader);
    //glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    //if (!success) ASSERT(false);

    //prog_id = glCreateProgram();
    //glAttachShader(prog_id, vert_shader);
    //glAttachShader(prog_id, frag_shader);
    //glLinkProgram(prog_id);
    //glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
    //if (!success) ASSERT(false);
}

texture_t* renderer_load_texture(platform_window_t* window, const char* filename)
{
    GLuint TextureID = 0;

    //stbi_set_flip_vertically_on_load(true); TODO doesn't work as expected

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) UNREACHABLE("image couldn't be loaded\n");

    int Mode = GL_RGB;
    switch (nrChannels)
    {
        case 2: { } break; // see down below
        case 3: { Mode = GL_RGB; } break;
        case 4: { Mode = GL_RGBA; } break;
        default: { UNREACHABLE("Unknown nr of channels '%i' for image '%s'\n", nrChannels, filename); break; }
    }

    // NOTE the greyscale texture (greyout.png) uses 2 channels (gray+alpha), which isn't supported by
    // (immediate-mode?) opengl, so as a workaround we load it in again & force it to 4 channels.
    // IMG_Load seemed to do that by default
    if (nrChannels == 2)
    {
        stbi_image_free(data);
        data = stbi_load(filename, &width, &height, &nrChannels, 4);
        Mode = GL_RGBA;
    }

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    // set some parameters TODO needed ?
    // set the texture wrapping parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, width, height, 0, Mode, GL_UNSIGNED_BYTE, data);

    // cleanup
    glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
    stbi_image_free(data);

    texture_t* tex = (texture_t*) malloc(sizeof(texture_t));
    tex->id     = TextureID;
    tex->width  = width;
    tex->height = height;

    return tex;
}

// TODO SDL specific
texture_t* renderer_create_texture_from_surface(platform_window_t* window, surface_t* surface)
{
    SDL_Surface* surf = (SDL_Surface*) surface;

    u32 tex_id = 0;

    int Mode = -1;
    switch (surf->format->BytesPerPixel)
    {
        case 3: { Mode = GL_RGB; } break;
        case 4: { Mode = GL_RGBA; } break;
        default: { UNREACHABLE("Unknown nr of bytes per pixel '%i'\n", surf->format->BytesPerPixel); break; }
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // set some parameters TODO needed ?
    // set the texture wrapping parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, surf->w, surf->h, 0, Mode, GL_UNSIGNED_BYTE, surf->pixels);

    // cleanup
    glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

    texture_t* tex = (texture_t*) malloc(sizeof(texture_t));
    tex->id     = tex_id;
    tex->width  = surf->w;
    tex->height = surf->h;
    return tex;
}

i32 renderer_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    // TODO handle nullpointers being passed
    *w = tex->width;
    *h = tex->height;
    return 0;
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

                if (draw_tex->tex == nullptr) // TODO font textures arent supported by this renderer
                {
                    curr_entry += sizeof(render_entry_texture_t);
                    break;
                }

                const u32 SCREEN_WIDTH = 1280; // TODO hardcoded
                const u32 SCREEN_HEIGHT = 960; // TODO hardcoded

                f32 TEXTURE_WIDTH  = draw_tex->tex->width;
                f32 TEXTURE_HEIGHT = draw_tex->tex->height;
                GLuint tex_id      = draw_tex->tex->id;

                // TODO we need to do this here because right now the game layer
                // can push empty src/dst rectangles, which is supposed to mean
                // to use the entire texture/entire screen.  Maybe we should
                // just disallow this, but then the game layer has to query for
                // texture attributes
                SDL_Rect* src = (SDL_Rect*) &draw_tex->src;
                SDL_Rect* dst = (SDL_Rect*) &draw_tex->dst;
                if (utils_rect_empty(draw_tex->src))
                {
                    src->x = 0;
                    src->y = 0;
                    src->w = TEXTURE_WIDTH;
                    src->h = TEXTURE_HEIGHT;
                }
                if (utils_rect_empty(draw_tex->dst))
                {
                    dst->x = 0;
                    dst->y = 0;
                    dst->w = SCREEN_WIDTH;
                    dst->h = SCREEN_HEIGHT;
                }

                // NOTE draw_tex->dst is in pixel coordinates (x,w:0-1280,
                // y,h:0-960), but opengl needs screen coordinates from -1 to 1
                // (origin is in the center of the screen)
                f32 screen_x = (draw_tex->dst.x / (SCREEN_WIDTH/2.f))  - 1.f;
                f32 screen_y = (draw_tex->dst.y / (SCREEN_HEIGHT/2.f)) - 1.f;
                f32 screen_w = (draw_tex->dst.w / (SCREEN_WIDTH/2.f));
                f32 screen_h = (draw_tex->dst.h / (SCREEN_HEIGHT/2.f));

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

                f32 vert_attribs[] =
                {
                    screen_x,            screen_y,            tex_x,         tex_y,         // vertex 1
                    screen_x + screen_w, screen_y,            tex_x + tex_w, tex_y,         // vertex 2
                    screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h, // vertex 3

                    screen_x,            screen_y,            tex_x,         tex_y,         // vertex 4
                    screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h, // vertex 5
                    screen_x,            screen_y + screen_h, tex_x,         tex_y + tex_h  // vertex 6
                };

                // TODO opengl code here
                glMatrixMode(GL_TEXTURE);
                glLoadIdentity();
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glScalef(1.0, -1.0, 1.0); // NOTE manually flip the image

                glViewport(0,0,1280,960); // TODO hardcoded

                glBindTexture(GL_TEXTURE_2D, tex_id);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

                //glUseProgram(prog_id);
                // pass texture as uniform to shader
                //glUniform1i(glGetUniformLocation(prog_id, "u_texture"), 0); // TODO why 0

                // create empty vao & bind (required by core opengl)
                //u32 vao;
                //glGenVertexArrays(1, &vao);
                //glBindVertexArray(vao);

                // create empty ibo & bind // TODO not needed?

                // create a vbo & bind & upload
                //u32 vbo;
                //glGenBuffers(1, &vbo);
                //glBindBuffer(GL_ARRAY_BUFFER, vbo);
                //glBufferData(GL_ARRAY_BUFFER, sizeof(vert_attribs), vert_attribs, GL_STATIC_DRAW);
                // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
                // GL_STATIC_DRAW: the data is set only once and used many times.
                // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

                // specify how vertices are laid out (TODO we don't need to do
                // this every frame if we just save this inside the vao & bound
                // the same vao every frame)
                //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*) 0);
                //glEnableVertexAttribArray(0);
                //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(f32)));
                //glEnableVertexAttribArray(1);

                // draw with ...
                //glDrawArrays(GL_TRIANGLES, 0, 6);

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

                // TODO unbind & delete buffers afterwards
                // glUseProgram(NULL);
                // glDeleteBuffers(1, &vbo);
                // glDeleteVertexArrays(1, &vao);

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

                /* TODO opengl code here */
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


// stubs
void renderer_destroy(renderer_t* renderer) {}
