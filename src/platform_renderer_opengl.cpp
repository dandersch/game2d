#include "platform_renderer.h"
#include "utils.h"

// unity build
#include "glew.c"

struct renderer_t
{
    SDL_GLContext gl_context;
};

// TODO test
// TODO there seems to be a built-in SDL function for what we do here:
//    SDL_GL_BindTexture(SDL_Texture *texture, float *texw, float *texh)
texture_t renderer_load_texture(platform_window_t* window, const char* filename)
{
    GLuint TextureID = 0;

    // You should probably use CSurface::OnLoad ... ;)
    //-- and make sure the Surface pointer is good!
    SDL_Surface* Surface = IMG_Load(filename);

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    int Mode = GL_RGB;
    if (Surface->format->BytesPerPixel == 4) Mode = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Any other glTex* stuff here

    // TODO do we have to unbind the texture here ?

    return TextureID; // TODO can we cast the uint here to a pointer w/o problems (32bit vs 64bit)?
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
    SDL_GLContext gl_context = SDL_GL_CreateContext(window->handle);
    SDL_ERROR(gl_context); // Failed to create OpenGL context.

    // init GLEW
    glewExperimental = GL_TRUE;
    i32 error        = glewInit();
    GLEW_ERROR(error);

    SDL_ERROR(!SDL_GL_SetSwapInterval(1)); // Couldn't set VSYNC

    // Generate program
    GLuint prog_id       = glCreateProgram();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    // set vertex shader source code
    const GLchar* vertex_shader_src[] = {
                                         "#version 140\n"
                                         "in vec2 pos;\n"
                                         "void main() {\n"
                                         "    gl_Position = vec4(pos.x, pos.y, 0, 1);\n"
                                         "}"
                                        };
    glShaderSource(vertex_shader, 1, vertex_shader_src, NULL); // Set vertex source
    glCompileShader(vertex_shader);                            // compile vertex source

    // check vertex shader for errors
    GLint compile_error = GL_FALSE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_error);
    if(compile_error != GL_TRUE)
    {
        printf("Unable to compile vertex shader %d!\n", vertex_shader);
        //printShaderLog(vertex_shader);
    }

    // attach vertex shader to program
    glAttachShader(prog_id, vertex_shader);

    // create fragment shader
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // get fragment source
    const GLchar* frag_shader_src[] = { "#version 140\n"
                                        "out vec4 LFragment;\n"
                                        "in vec2 pos;\n"
                                        "void main() {\n"
                                        "    LFragment = vec4(pos.x, pos.y, 1.0, 1.0 );\n"
                                        "}" };

    // set fragment source
    glShaderSource(frag_shader, 1, frag_shader_src, NULL );

    // compile fragment source
    glCompileShader(frag_shader);

    // check fragment shader for errors
    compile_error = GL_FALSE;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &compile_error);
    if(compile_error != GL_TRUE)
    {
        printf( "Unable to compile fragment shader %d!\n", frag_shader);
        //printShaderLog( fragmentShader );
    }

    // attach fragment shader to program
    glAttachShader(prog_id, frag_shader);

    // link program
    glLinkProgram(prog_id);

    // check for errors
    GLint prog_success = GL_TRUE;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &prog_success);
    if(prog_success != GL_TRUE)
    {
        printf("Error linking program %d!\n", prog_id);
        // printProgramLog( gProgramID );
    }

    //Get vertex attribute location
    GLint gVertexPos2DLocation = -1;
    gVertexPos2DLocation = glGetAttribLocation(prog_id, "pos");
    if(gVertexPos2DLocation == -1)
    {
        printf("pos is not a valid glsl program variable!\n");
    }

    GLuint gVBO = 0;
    GLuint gIBO = 0;

    // Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );

    // VBO data
    GLfloat vertexData[] = { -0.5f, -0.5f,
                              0.5f, -0.5f,
                              0.5f,  0.5f, };

    // IBO data
    GLuint indexData[] = { 0, 1, 2 };

    // generate vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create VBO
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * (sizeof(vertexData)/sizeof(*vertexData)) * sizeof(GLfloat),
                 vertexData, GL_STATIC_DRAW);

    // create IBO
    glGenBuffers(1, &gIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (sizeof(indexData)/sizeof(*indexData)) * sizeof(GLuint),
                 indexData, GL_STATIC_DRAW);

    // test loading in a texture (seems to work)
    GLuint tex_id = renderer_load_texture(window, "res/character.png");
    float texCoords[] = { 0.0f, 0.0f,  // bottom left
                          1.0f, 0.0f,  // bottom right
                          1.0f, 1.0f,  // top right
                          0.0f, 1.0f   // top left
                         };


    u32 counter = 0;
    while (counter < 100)
    {
        glClearColor(1.0f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);      // clear color buffer

        glUseProgram(prog_id); // Bind program
        glEnableVertexAttribArray(gVertexPos2DLocation); // Enable vertex position

        // set vertex data
        glBindBuffer(GL_ARRAY_BUFFER, gVBO);
        glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);

        // set index data and render
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

        // Disable vertex position
        glDisableVertexAttribArray(gVertexPos2DLocation);

        // Unbind program
        glUseProgram(NULL);

        // TODO try drawing the texture

        SDL_GL_SwapWindow(window->handle);

        counter++;
        printf("%u\n", counter);
    }

    exit(1);
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

                // TODO opengl code here
                glBindTexture(GL_TEXTURE_2D, (GLuint) draw_tex->tex);
                // ...

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

                // TODO opengl code her
                // ...

                curr_entry += sizeof(render_entry_clear_t);
            } break;

            case RENDER_ENTRY_TYPE_PRESENT:
            {
                curr_entry += sizeof(render_entry_header_t);

                // TODO opengl code her
                // ...
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
