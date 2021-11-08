#include "platform_renderer.h"
#include "utils.h"
#include "memory.h"

/*  possible optimizations/improvements for this renderer:
 *  [ ] better opengl error handling
 *  [ ] use of index buffer objects (ibo's)
 *  [X] use binding of more textures/texture units & pass a tex_idx to the fragment shader so
 *      that it can dynamically sample from the right texture
 *  [ ] tilemap culling (should probably happen in the game layer)
 *  [ ] use glBufferData(GL_ARRAY_BUFFER, sizeof(batch), NULL, GL_STATIC_DRAW)
 *      to allocate memory on the gpu beforehand
 *  [ ] use instancing
 */

struct renderer_t
{
    void* gl_context;
};

struct texture_t
{
    u32 id;
    u32 width;
    u32 height;
    f32 unit_idx; // NOTE unused
};

#if 0 // SMOOTH PIXEL ZOOM see https://www.shadertoy.com/view/MlB3D3
const float pixelSize =    4.00; // "fat pixel" size
const float zoom      =    8.00; // max zoom factor
const float radius    =  128.00; // planar movement radius
const float speed     =    0.25; // speed
void mainImage( out vec4 color, in vec2 pixel )
{
    // zoom & scroll
    float time   = iTime * speed;
    float scale  = pixelSize + ((cos((time + 8.0) / 3.7) + 1.0) / 2.0) * (zoom - 1.0) * pixelSize;
    vec2  center = vec2(-4.0, 16.0) + iResolution.xy / 2.0;
    vec2  offset = vec2(cos(time), sin(time)) * radius;

    pixel = ((pixel + offset) - center) / scale + center;

    // emulate point sampling
    vec2 uv = floor(pixel) + 0.5;

    // subpixel aa algorithm (COMMENT OUT TO COMPARE WITH POINT SAMPLING)
    uv += 1.0 - clamp((1.0 - fract(pixel)) * scale, 0.0, 1.0);

    // output
    color = texture(iChannel0, uv / iChannelResolution[0].xy);
}
#endif

// TODO pass in or calculate a zoom factor
const char* vertex_shader_src =
    "#version 330 core\n"
    "uniform mat4  u_camera;\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 tex_coords;\n"
    "layout (location = 2) in float tex_index;\n"
    "layout (location = 3) in vec4 color;\n"
    "out vec2 o_tex_coords;\n"
    "out float o_tex_index;\n"
    "out vec4 o_color;\n"
    "void main()\n"
    "{\n"
        "gl_Position  = u_camera * vec4(pos.x, pos.y, 1.0, 1.0);\n" // NOTE -y seems to fix orientation
        "o_tex_coords = tex_coords;\n"
        "o_tex_index  = tex_index;\n"
        "o_color      = color;\n"
    "}\0";
const char* fragment_shader_src =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 o_tex_coords;\n"
    "in float o_tex_index;\n"
    "in vec4 o_color;\n"
    "uniform sampler2D u_tex_units[16];\n"
    "uniform float u_zoom;\n"
    "void main()\n"
    "{\n"

        //"const float pixelSize =    4.00; // 'fat pixel' size\n"
        //"float scale  = pixelSize + (zoom - 1.0) * pixelSize;\n"
        //"vec2  center = vec2(-4.0, 16.0) + iResolution.xy / 2.0;\n"
        //"vec2  pixel  = (pixel - center) / scale + center;\n"
        //"vec2 uv = floor(pixel) + 0.5;\n"
        //"uv += 1.0 - clamp((1.0 - fract(pixel)) * scale, 0.0, 1.0);"

        "FragColor = o_color;\n"
        "switch(int(o_tex_index))\n"
        "{\n"
            "case  0:                                                    ; break;\n"
            "case  1: FragColor *= texture(u_tex_units[ 1], o_tex_coords); break;\n"
            "case  2: FragColor *= texture(u_tex_units[ 2], o_tex_coords); break;\n"
            "case  3: FragColor *= texture(u_tex_units[ 3], o_tex_coords); break;\n"
            "case  4: FragColor *= texture(u_tex_units[ 4], o_tex_coords); break;\n"
            "case  5: FragColor *= texture(u_tex_units[ 5], o_tex_coords); break;\n"
            "case  6: FragColor *= texture(u_tex_units[ 6], o_tex_coords); break;\n"
            "case  7: FragColor *= texture(u_tex_units[ 7], o_tex_coords); break;\n"
            "case  8: FragColor *= texture(u_tex_units[ 8], o_tex_coords); break;\n"
            "case  9: FragColor *= texture(u_tex_units[ 9], o_tex_coords); break;\n"
            "case 10: FragColor *= texture(u_tex_units[10], o_tex_coords); break;\n"
            "case 11: FragColor *= texture(u_tex_units[11], o_tex_coords); break;\n"
            "case 12: FragColor *= texture(u_tex_units[12], o_tex_coords); break;\n"
            "case 13: FragColor *= texture(u_tex_units[13], o_tex_coords); break;\n"
            "case 14: FragColor *= texture(u_tex_units[14], o_tex_coords); break;\n"
            "case 15: FragColor *= texture(u_tex_units[15], o_tex_coords); break;\n"
        "}\n"
    "}\0";

struct vertex_t
{
    f32 vert_x, vert_y;
    f32 tex_x,  tex_y;
    f32 tex_idx;
    colorf_t color;
};
#define BATCHED_VERTICES_MAX 50000 // batch gets flushed if it exceeds this max
global_var vertex_t* vbo_batch;    // TODO put this in a frame_arena (?)
global_var u32 vbo;
global_var u32 vertex_count = 0;
global_var u32 prog_id;
#define MAX_TEX_UNITS 16 // NOTE also needs to be changed in fragment shader if changed!
global_var i32 uni_loc_tex_units;
global_var i32 uni_loc_camera;
global_var i32 uni_loc_zoom;
global_var u32 vao;

// GLEW_OK = 0
#define GLEW_ERROR(x) if(x) printf("Error initializing GLEW! %s\n", glewGetErrorString(x));

void renderer_init(platform_window_t* window, mem_arena_t* platform_mem_arena)
{
    // TODO use a COMMON_RENDERER_INIT macro for this
    sort_entry_count  = 0;
    cmds = (renderer_cmd_buf_t*) mem_arena_alloc(platform_mem_arena, sizeof(cmds));
    cmds->buf_offset  = cmds->buf;
    cmds->entry_count = 0;

    /* opengl code here */

    // init GLEW
    glewExperimental = GL_TRUE;
    i32 error        = glewInit();
    GLEW_ERROR(error);

    // 1 to set vsync, 0 to ensure it's off
    // NOTE setting vsync off gives a much smoother experience right now, which might be caused by
    // our weird timestepping TODO investigate
    SDL_ERROR(!SDL_GL_SetSwapInterval(0)); // error if couldn't set

    printf("%s\n", glGetString(GL_VERSION));

    // CREATE SHADERS & PROGRAM
    i32  success;
    char info_log[512];

    u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vert_shader);
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(prog_id, 512, NULL, info_log);
        printf("%s\n", info_log);
        UNREACHABLE("couldn't compile vertex shader\n");
    }

    u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(prog_id, 512, NULL, info_log);
        printf("%s\n", info_log);
        UNREACHABLE("couldn't compile fragment shader\n");
    }

    prog_id = glCreateProgram();
    glAttachShader(prog_id, vert_shader);
    glAttachShader(prog_id, frag_shader);
    glLinkProgram(prog_id);
    glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(prog_id, 512, NULL, info_log);
        printf("%s\n", info_log);
        UNREACHABLE("couldn't link program\n");
    }

    // cache uniform location
    uni_loc_camera = glGetUniformLocation(prog_id, "u_camera");
    if (uni_loc_camera == -1) { UNREACHABLE("uniform '%s' not found\n", "u_camera"); }

    //uni_loc_zoom = glGetUniformLocation(prog_id, "u_zoom");
    //if (uni_loc_zoom == -1) { UNREACHABLE("uniform '%s' not found\n", "u_zoom"); }

    uni_loc_tex_units = glGetUniformLocation(prog_id, "u_tex_units");
    if (uni_loc_tex_units == -1) { UNREACHABLE("uniform '%s' not found\n", "u_tex_units"); }
    i32 samplers[MAX_TEX_UNITS] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    glUseProgram(prog_id); // NOTE needs to be set before uploading uniforms

    // upload uniforms
    f32 cam_mtx[4][4] = {{1,0,0,0},
                         {0,1,0,0},
                         {0,0,1,0},
                         {0,0,0,1}};
    glUniformMatrix4fv(uni_loc_camera, 1, GL_FALSE, &cam_mtx[0][0]);
    glUniform1iv(uni_loc_tex_units, 16, samplers);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * BATCHED_VERTICES_MAX, nullptr, GL_DYNAMIC_DRAW);
    // TODO use different draw:
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    vbo_batch = (vertex_t*) malloc(BATCHED_VERTICES_MAX * sizeof(vertex_t));

    //glEnable(GL_TEXTURE_2D); // NOTE causes unknown error
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO maybe bind a 'missing' texture on texture unit 0 for debug purposes
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, missing_tex_id);

    auto err = glGetError();
    if (err != GL_NO_ERROR) printf("%s\n", glewGetErrorString(err));
}


void renderer_upload_camera(cam_mtx_t mtx, f32 zoom)
{
    glUseProgram(prog_id);
    glUniformMatrix4fv(uni_loc_camera, 1, GL_FALSE, &mtx.mtx[0][0]);

    //glUniform1fv(uni_loc_camera, 1, &zoom);
}


internal_fn texture_t* create_and_upload_texture(i32 width, i32 height, i32 mode, void* data)
{
    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glActiveTexture(GL_TEXTURE0 + tex_id); // NOTE TEXTURE0 is actually unused this way, so right now
                                           // we don't sample any texture when tex_id = 0
    glBindTexture(GL_TEXTURE_2D, tex_id);

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

    glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, data);

    texture_t* tex = (texture_t*) malloc(sizeof(texture_t));
    tex->id        = tex_id;
    tex->width     = width;
    tex->height    = height;

    return tex;
}


texture_t* renderer_load_texture(platform_window_t* window, const char* filename)
{
    //stbi_set_flip_vertically_on_load(true); // TODO doesn't work as expected

    i32 width, height, nrChannels;
    u8* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) UNREACHABLE("image '%s' couldn't be loaded\n", filename);

    i32 Mode = GL_RGB;
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

    texture_t* tex = create_and_upload_texture(width, height, Mode, data);

    // cleanup
    stbi_image_free(data);
    //glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

    return tex;
}


// TODO SDL specific, we could instead have our own definition of surface_t
// inside the renderer, which could contain e.g. the data from stbi_load
// NOTE hasn't been tested for a while
texture_t* renderer_create_texture_from_surface(platform_window_t* window, surface_t* surface)
{
    SDL_Surface* surf = (SDL_Surface*) surface;

    i32 Mode = -1;
    switch (surf->format->BytesPerPixel)
    {
        case 3: { Mode = GL_RGB; } break;
        case 4: { Mode = GL_RGBA; } break;
        default: { UNREACHABLE("Unknown nr of bytes per pixel '%i'\n", surf->format->BytesPerPixel); break; }
    }

    texture_t* tex = create_and_upload_texture(surf->w, surf->h, Mode, surf->pixels);

    // cleanup
    //glBindTexture(GL_TEXTURE_2D, 0); // unbind texture

    return tex;
}


i32 renderer_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    // TODO handle nullpointers being passed
    *w = tex->width;
    *h = tex->height;
    return 0;
}


void flush_batch()
{
    glUseProgram(prog_id);

    // create empty ibo & bind NOTE not needed right now
    //u32 ibo;
    //glGenVertexArrays(1, &ibo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // upload to already allocated vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * vertex_count, vbo_batch);

    // create vao & bind (required by core opengl)
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // specify how vertices are laid out (TODO we don't need to do this every frame if we just save this
    // inside the vao & bound the same vao every frame NOTE that doesn't seem to work...)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, vert_x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, tex_x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, tex_idx));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, color));
    glEnableVertexAttribArray(3);

    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // unbind & delete buffers afterwards
    // NOTE causes a crash if we don't do this
    glUseProgram(NULL);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    vertex_count = 0;
}


void renderer_cmd_buf_process(platform_window_t* window)
{
    renderer_sort_buffer();

    for (u32 i = 0; i < sort_entry_count; i++)
    {
        u8* curr_entry = (u8*) sort_buf[i].cmd_entry;
        render_entry_header_t* entry_header = (render_entry_header_t*) curr_entry;
        curr_entry += sizeof(render_entry_header_t);
        switch (entry_header->type)
        {
            case RENDER_ENTRY_TYPE_TEXTURE:
            {
                render_entry_texture_t* draw_tex = (render_entry_texture_t*) curr_entry;

                ASSERT(draw_tex->tex);
                ASSERT(!utils_rect_empty(draw_tex->dst));
                ASSERT(!utils_rect_empty(draw_tex->src));

                const u32 SCREEN_WIDTH  = window->width;
                const u32 SCREEN_HEIGHT = window->height;

                const f32 TEXTURE_WIDTH  = draw_tex->tex->width;
                const f32 TEXTURE_HEIGHT = draw_tex->tex->height;
                f32 tex_id         = (f32) draw_tex->tex->id;
                colorf_t color     = {1,1,1,1};

                // NOTE draw_tex->dst is in pixel coordinates (x,w:0-1280, y,h:0-960),
                // but opengl needs screen coordinates from -1 to 1 (origin is in the center of the screen)
                //f32 screen_x = (draw_tex->dst.left / (SCREEN_WIDTH/2.f))  - 1.f;
                //f32 screen_y = MAP_VALUE_IN_RANGE1_TO_RANGE2(draw_tex->dst.top, 0.0f, SCREEN_HEIGHT, 1.0f, -1.0f); // map to -1 to 1
                //f32 screen_w = (draw_tex->dst.w / (SCREEN_WIDTH/2.f));
                //f32 screen_h = -(draw_tex->dst.h / (SCREEN_HEIGHT/2.f));
                f32 screen_x = draw_tex->dst.left;
                f32 screen_y = draw_tex->dst.top; // map to -1 to 1
                f32 screen_w = draw_tex->dst.w;
                f32 screen_h = draw_tex->dst.h;

                // NOTE draw_tex->src is in pixel coordinates
                // (x,w:0-texture_width y,h:0-texture_height with origin in top left corner),
                // but opengl needs texture coordinates from 0 to 1 (origin is bottom left corner)
                f32 tex_x = (draw_tex->src.left / TEXTURE_WIDTH) ;
                f32 tex_y = (draw_tex->src.top  / TEXTURE_HEIGHT) - 1.0f;
                f32 tex_w = (draw_tex->src.w    / TEXTURE_WIDTH);
                f32 tex_h = (draw_tex->src.h    / TEXTURE_HEIGHT);

                // flush the batch if we don't have any room left for another quad
                if (vertex_count + 6 >= BATCHED_VERTICES_MAX) flush_batch();

                { // add to batch
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            tex_x,         tex_y,          tex_id, color // vertex 1
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y,            tex_x + tex_w, tex_y,          tex_id, color // vertex 2
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h,  tex_id, color // vertex 3
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            tex_x,         tex_y,          tex_id, color // vertex 1
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h,  tex_id, color // vertex 3
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y + screen_h, tex_x,         tex_y + tex_h,  tex_id, color // vertex 4
                    };
                }

                /* opengl code here */
                //glViewport(0,0,1280,960); // TODO hardcoded, doesn't seem to do anything in core profile

                curr_entry += sizeof(render_entry_texture_t);
            } break;

            case RENDER_ENTRY_TYPE_RECT:
            {
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;

                const u32 SCREEN_WIDTH  = window->width;
                const u32 SCREEN_HEIGHT = window->height;
                colorf_t color = rect->color;

                // NOTE rect is in pixel coordinates (x,w:0-1280, y,h:0-960),
                // but opengl needs screen coordinates from -1 to 1 (origin is in the center of the screen)
                // TODO duplicated
                // f32 screen_x = (rect->rect.left / (SCREEN_WIDTH/2.f))  - 1.f;
                // f32 screen_y = MAP_VALUE_IN_RANGE1_TO_RANGE2(rect->rect.top, 0.0f, SCREEN_HEIGHT, 1.0f, -1.0f); // map to -1 to 1
                // f32 screen_w = (rect->rect.w / (SCREEN_WIDTH/2.f));
                // f32 screen_h = -(rect->rect.h / (SCREEN_HEIGHT/2.f));
                f32 screen_x = rect->rect.left;
                f32 screen_y = rect->rect.top; // map to -1 to 1
                f32 screen_w = rect->rect.w;
                f32 screen_h = rect->rect.h;


                // flush the batch if we don't have any room left for another quad
                if (vertex_count + 6 >= BATCHED_VERTICES_MAX) flush_batch();

                { // add to batch
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            0, 0,  0, color // vertex 1
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y,            0, 0,  0, color // vertex 2
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, 0, 0,  0, color // vertex 3
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            0, 0,  0, color // vertex 1
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, 0, 0,  0, color // vertex 3
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y + screen_h, 0, 0,  0, color // vertex 4
                    };
                }

                curr_entry += sizeof(render_entry_rect_t);
            } break;

            case RENDER_ENTRY_TYPE_TEXTURE_MOD:
            {
                render_entry_texture_mod_t* mod = (render_entry_texture_mod_t*) curr_entry;

                // TODO opengl code here
                // ...

                curr_entry += sizeof(render_entry_texture_mod_t);
            } break;

            case RENDER_ENTRY_TYPE_CLEAR:
            {
                /* opengl code here */
                glClearColor(0.0f,0.0f,0.0f,1.0f);
                glClear(GL_COLOR_BUFFER_BIT);      // clear color buffer

                curr_entry += sizeof(render_entry_clear_t);
            } break;

            case RENDER_ENTRY_TYPE_PRESENT:
            {
                flush_batch();

                SDL_GL_SwapWindow(window->handle);

                curr_entry += sizeof(render_entry_present_t);
            } break;

            default:
            {
                UNREACHABLE("OpenGL render command for entry '%u' not implemented\n", entry_header->type);
            }
        }
    }
    // reset cmd buffer
    sort_entry_count  = 0;
    cmds->entry_count = 0;
    cmds->buf_offset  = cmds->buf;
}

// stubs
void renderer_destroy(renderer_t* renderer) {}
