#include "platform_renderer.h"
#include "utils.h"

// unity build
#include "glew.c"

struct renderer_t
{
    SDL_GLContext gl_context;
};

// TODO temp globals
global GLuint prog_id;
global GLuint tex_id;
global GLuint vao, vbo, ibo;

// TODO test
texture_t renderer_load_texture(platform_window_t* window, const char* filename)
{
    // TODO there seems to be a built-in SDL function for what we do here:
    //    SDL_GL_BindTexture(SDL_Texture *texture, float *texw, float *texh)
    // "You need a renderer to create an SDL_Texture, therefore you can only use
    //  this function with an implicit OpenGL context from SDL_CreateRenderer(),
    //  not with your own OpenGL context. If you need control over your OpenGL
    //  context, you need to write your own texture-loading methods."
    //window->renderer = (void*) SDL_CreateRenderer(window->handle, -1, SDL_RENDERER_ACCELERATED);
    //SDL_GL_BindTexture(SDL_Texture*, w, h);

    GLuint TextureID = 0;

    SDL_Surface* Surface = IMG_Load(filename);

    // TODO make surface upside down (origin in SDL_image is upper left, origin in opengl is bottom left...)
    // OR use stb_image.h

    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);


    // set some parameters TODO needed ?
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int Mode = GL_RGB;
    if (Surface->format->BytesPerPixel == 4) Mode = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, Surface->w, Surface->h, 0, Mode, GL_UNSIGNED_BYTE, Surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Any other glTex* stuff here

    // cleanup
    // TODO do we have to unbind the texture here ?
    // DestroySurface(Surface);

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
    SDL_GLContext gl_context = SDL_GL_CreateContext(window->handle);
    SDL_ERROR(gl_context); // Failed to create OpenGL context.

    // init GLEW
    glewExperimental = GL_TRUE;
    i32 error        = glewInit();
    GLEW_ERROR(error);

    SDL_ERROR(!SDL_GL_SetSwapInterval(1)); // Couldn't set VSYNC

    printf("%s\n", glGetString(GL_VERSION));

    // CREATE PROGRAM /////////////////////////////////////////////////////////////////////////////
    prog_id       = glCreateProgram(); // Generate program
    // set vertex shader source code
    const GLchar* vertex_shader_src[] = {
                                          "#version 330 core\n"
                                          "layout (location = 0) in vec3 aPos;\n"
                                          "layout (location = 1) in vec3 aColor;\n"
                                          "layout (location = 2) in vec2 aTexCoord;\n"
                                          "out vec3 ourColor;\n"
                                          "out vec2 TexCoord;\n"
                                          "uniform vec2 transl;\n"
                                          "uniform vec2 tex_coords;\n"
                                          "void main() {\n"
                                          "    gl_Position = vec4(aPos.x + transl.x, aPos.y + transl.y, aPos.z, 1);\n"
                                          "    ourColor = aColor;\n"
                                          "    TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
                                          "}"
                                        };
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);   // generate vertex shader
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
    glAttachShader(prog_id, vertex_shader); // attach vertex shader to program
    // set fragment shader source code
    const GLchar* frag_shader_src[] = { "#version 330 core\n"
                                        "out vec4 FragColor;\n"
                                        "in vec3 ourColor;\n"
                                        "in vec2 TexCoord;\n"
                                        "uniform sampler2D ourTexture;\n"
                                        "void main()\n"
                                        "{\n"
                                        "    FragColor = texture(ourTexture, TexCoord);\n"
                                        "}"};
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER); // create fragment shader
    glShaderSource(frag_shader, 1, frag_shader_src, NULL );  // set fragment source
    glCompileShader(frag_shader);                            // compile fragment source
    // check fragment shader for errors
    compile_error = GL_FALSE;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &compile_error);
    if(compile_error != GL_TRUE)
    {
        printf( "Unable to compile fragment shader %d!\n", frag_shader);
        //printShaderLog( fragmentShader );
    }
    glAttachShader(prog_id, frag_shader); // attach fragment shader to program
    glLinkProgram(prog_id);               // link program
    // check for errors
    GLint prog_success = GL_TRUE;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &prog_success);
    if(prog_success != GL_TRUE)
    {
        printf("Error linking program %d!\n", prog_id);
        // printProgramLog( gProgramID );
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////

    // GENERATE OBJECTS //////////////////////////////////////////////////////////////////////////////
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    // generate objects
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //////////////////////////////////////////////////////////////////////////////////////////////////

    // LOAD IN TEXTURE ///////////////////////////////////////////////////////////////////////////////
    tex_id = renderer_load_texture(window, "res/gravetiles.png");
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(prog_id); // Bind program before settings uniforms!

    glUniform1i(glGetUniformLocation(prog_id, "ourTexture"), 0); // TODO why 0
    glUniform2f(glGetUniformLocation(prog_id, "transl"), 0.5f, 0.5f);

#if 1
    u32 counter = 0;
    while (counter < 100)
    {
        glClearColor(1.0f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);      // clear color buffer

        // draw the texture
        glBindTexture(GL_TEXTURE_2D, tex_id);

        glUseProgram(prog_id); // Bind program
        glUniform2f(glGetUniformLocation(prog_id, "transl"), counter/100.f, counter/100.f);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        SDL_GL_SwapWindow(window->handle);

        counter++;
        printf("%u\n", counter);
    }
    exit(0);
#endif
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

                // TODO this is hardcoded for testing, we should make texture_t
                // an opaque ptr instead that is defined by the renderer and in
                // the case of opengl should at least contain the GLuint tex_id
                // & width & height
                const u32 TEXTURE_WIDTH  = 352;
                const u32 TEXTURE_HEIGHT = 224;

                // NOTE draw_tex->dst is in pixel coordinates (x,w:0-1280,
                // y,h:0-960), but opengl needs screen coordinates from -1 to 1
                // (origin is in the center of the screen)
                f32 screen_x = (draw_tex->dst.x / (SCREEN_WIDTH/2))  - 1;
                f32 screen_y = (draw_tex->dst.y / (SCREEN_HEIGHT/2)) - 1;
                f32 screen_w = (draw_tex->dst.w / (SCREEN_WIDTH/2));
                f32 screen_h = (draw_tex->dst.w / (SCREEN_HEIGHT/2));

                // NOTE draw_tex->src is in pixel coordinates
                // (x,w:0-texture_width y,h:0-texture_height), but opengl needs
                // texture coordinates from 0 to 1 (origin is bottom left corner)
                f32 tex_x = (draw_tex->src.x / (TEXTURE_WIDTH/2));
                f32 tex_y = (draw_tex->src.y / (TEXTURE_HEIGHT/2));
                f32 tex_w = (draw_tex->src.w / (TEXTURE_WIDTH/2));
                f32 tex_h = (draw_tex->src.w / (TEXTURE_HEIGHT/2));

                // TODO opengl code here
                const float verts[] = {
                    screen_x,            screen_y,
                    screen_y + screen_w, screen_y,
                    screen_x + screen_w, screen_y + screen_h,
                    screen_x,            screen_y + screen_h
                };
                //const float tw = float(spriteWidth) / texWidth;
                //const float th = float(spriteHeight) / texHeight;
                //const int numPerRow = texWidth / spriteWidth;
                //const float tx = (frameIndex % numPerRow) * tw;
                //const float ty = (frameIndex / numPerRow + 1) * th;
                const float texVerts[] = {
                    tex_x,         tex_y,
                    tex_x + tex_w, tex_y,
                    tex_x + tex_w, tex_y + tex_h,
                    tex_x,         tex_y + tex_h
                };

                // ... Bind the texture, enable the proper arrays
                glBindTexture(GL_TEXTURE_2D, (GLuint) draw_tex->tex);
                glUseProgram(prog_id);
                glUniform1i(glGetUniformLocation(prog_id, "ourTexture"), 0); // TODO why 0

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

                glVertexPointer(2, GL_FLOAT, 0, verts);
                glTexCoordPointer(2, GL_FLOAT, 0, texVerts);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
                glClearColor(1.0f,0.1f,0.1f,1.0f);
                glClear(GL_COLOR_BUFFER_BIT);      // clear color buffer

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
