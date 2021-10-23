#include "platform_renderer.h"
#include "utils.h"
#include "memory.h"

// unity build
// ...

/*  possible optimizations for this renderer
 *  - better batch rendering
 *  - use binding of more textures/texture units & pass a tex_idx to the fragment shader so
 *    that it can dynamically sample from the right texture
 *  - texture arrays
 *  - use of index buffer objects (ibo's)
 *  - tilemap culling (should probably happen in the game layer)
 *  - use glBufferData(GL_ARRAY_BUFFER, sizeof(batch), NULL, GL_STATIC_DRAW)
 *    to allocate memory on the gpu beforehand
 */

struct renderer_t
{
    SDL_GLContext gl_context;
};

struct texture_t
{
    u32 id;
    u32 width;
    u32 height;
    // u32 unit_idx;
    // ...
};

#define MAX_TEX_UNITS 16
const char* vertex_shader_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 tex_coords;\n"
    "out vec2 o_tex_coords;\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(pos.x, -pos.y, 1.0, 1.0);\n" // NOTE -y seems to fix orientation
        "o_tex_coords = tex_coords;\n"
    "}\0";
const char* fragment_shader_src =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 o_tex_coords;\n"
    //"in float tex_index;\n"
    "uniform sampler2D u_texture;\n"
    //"uniform sampler2D u_tex_units[16];\n" // TODO
    //"uniform sampler2DArray u_tex_array;\n" // TODO
    "void main()\n"
    "{\n"
        "FragColor = texture(u_texture, o_tex_coords);\n"
    "}\0";
global_var u32 prog_id;
global_var i32 uniform_loc;
global_var i32 uni_loc_tex_units;

struct vertex_t
{
    f32 vert_x, vert_y;
    f32 tex_x,  tex_y;
};

#define BATCHED_VERTICES_MAX 50000 // TODO just flush the batch right before we would exceed this max
global_var vertex_t* vbo_batch; // TODO find out good max size TODO put this in a frame_arena (?)
global_var u32 vertex_count = 0;

global_var u32 tex_array_id; // TODO use 1024x1024 textures for everything so that we can use texture arrays

global_var renderer_cmd_buf_t* cmds;

global_var u32 vao;

// GLEW_OK = 0
#define GLEW_ERROR(x) if(x) printf("Error initializing GLEW! %s\n", glewGetErrorString(x));

void renderer_init(platform_window_t* window, mem_arena_t* platform_mem_arena)
{
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

    i32 success;
    u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vert_shader);
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success) { UNREACHABLE("couldn't compile vertex shader\n"); }

    u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success) { UNREACHABLE("couldn't compile fragment shader\n"); }

    prog_id = glCreateProgram();
    glAttachShader(prog_id, vert_shader);
    glAttachShader(prog_id, frag_shader);
    glLinkProgram(prog_id);
    glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
    if (!success) { UNREACHABLE("couldn't link program\n"); }

    // cache uniform location
    uniform_loc = glGetUniformLocation(prog_id, "u_texture");
    if (uniform_loc == -1) { UNREACHABLE("uniform '%s' not found\n", "u_texture"); }
    //uni_loc_tex_units = glGetUniformLocation(prog_id, "u_tex_units");
    //if (uni_loc_tex_units == -1) { UNREACHABLE("uniform '%s' not found\n", "u_tex_units"); }
    //i32 samplers[MAX_TEX_UNITS] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    //glUniform1iv(uni_loc_tex_units, 16, samplers);

    vbo_batch = (vertex_t*) malloc(BATCHED_VERTICES_MAX * sizeof(vertex_t));

    // we use a texture array & fill it later with loaded textures
    //glGenTextures(1, &tex_array_id);
    //glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array_id);

    // allocate storage for the array
    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);
}

texture_t* renderer_load_texture(platform_window_t* window, const char* filename)
{
    GLuint TextureID = 0;

    //stbi_set_flip_vertically_on_load(true); TODO doesn't work as expected

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) UNREACHABLE("image '%s' couldn't be loaded\n", filename);

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
    // glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

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

// TODO SDL specific, we could instead have our own definition of surface_t
// inside the renderer, which could contain e.g. the data from stbi_load
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

    // TODO duplicated:
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

// NOTE right now the batched render only is called for tiles w/ the tiles texture.
// We can either...
// - merge all textures into one (i.e. only one .png), so that we never have to bind another texture
// - use texture arrays (all .pngs must then be the same size or resized when loading them in)
// - keep a texture id inside the vbo for every quad bind several textures (glBindTextureUnit(0, tex_id)) & keep t
// - have a 'texture change lookup table' that tells us at which vertex we have to bind another texture
// - implement a texture atlas
void flush_batch()
{
    glUseProgram(prog_id);
    // pass texture as uniform to shader
    glUniform1i(uniform_loc, 0); // TODO why 0

    // create empty ibo & bind NOTE not needed right now
    //u32 ibo;
    //glGenVertexArrays(1, &ibo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // create a vbo & bind & upload
    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * vertex_count, vbo_batch, GL_STATIC_DRAW);
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    // create vao & bind (required by core opengl)
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // specify how vertices are laid out (TODO we don't need to do this every frame if we just save this
    // inside the vao & bound the same vao every frame NOTE that doesn't seem to work...)
    // TODO stride = sizeof(vertex_attribute_t), offset = use offsetof
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, vert_x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) offsetof(vertex_t, tex_x));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

    // unbind & delete buffers afterwards NOTE doesn't seem to make a difference
    glUseProgram(NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    vertex_count = 0;
}

void renderer_cmd_buf_process(platform_window_t* window)
{
    u8* curr_entry = cmds->buf;
    for (s32 entry_nr = 0; entry_nr < cmds->entry_count; ++entry_nr)
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

                f32 TEXTURE_WIDTH  = draw_tex->tex->width;
                f32 TEXTURE_HEIGHT = draw_tex->tex->height;
                GLuint tex_id      = draw_tex->tex->id;

                // TODO we need to do this here because right now the game layer can push empty src/dst rectangles,
                // which is supposed to mean to use the entire texture/entire screen.
                // Maybe we should just disallow this, but then the game layer has to query for texture attributes
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

                // NOTE draw_tex->dst is in pixel coordinates (x,w:0-1280, y,h:0-960),
                // but opengl needs screen coordinates from -1 to 1 (origin is in the center of the screen)
                f32 screen_x = (draw_tex->dst.x / (SCREEN_WIDTH/2.f))  - 1.f;
                f32 screen_y = (draw_tex->dst.y / (SCREEN_HEIGHT/2.f)) - 1.f;
                f32 screen_w = (draw_tex->dst.w / (SCREEN_WIDTH/2.f));
                f32 screen_h = (draw_tex->dst.h / (SCREEN_HEIGHT/2.f));

                // NOTE draw_tex->src is in pixel coordinates
                // (x,w:0-texture_width y,h:0-texture_height with origin in top left corner),
                // but opengl needs texture coordinates from 0 to 1 (origin is bottom left corner)
                // TODO change origin
                f32 tex_x = (draw_tex->src.x / TEXTURE_WIDTH);
                f32 tex_y = (draw_tex->src.y / TEXTURE_HEIGHT);
                f32 tex_w = (draw_tex->src.w / TEXTURE_WIDTH);
                f32 tex_h = (draw_tex->src.h / TEXTURE_HEIGHT);

                // flush the batch if we don't have any room left for another quad
                if (vertex_count + 6 >= BATCHED_VERTICES_MAX)
                    flush_batch();

                // TODO support batching w/ different textures
                // NOTE we flush the current batch if the main texture changed for now
                if (tex_id != 1)
                    flush_batch();

                { // add to batch
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            tex_x,         tex_y,         // vertex 1
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y,            tex_x + tex_w, tex_y,         // vertex 2
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h, // vertex 3
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y,            tex_x,         tex_y,         // vertex 4
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x + screen_w, screen_y + screen_h, tex_x + tex_w, tex_y + tex_h, // vertex 5
                    };
                    vbo_batch[vertex_count++] = {
                        screen_x,            screen_y + screen_h, tex_x,         tex_y + tex_h  // vertex 6
                    };
                }

                /* opengl code here */
                //glViewport(0,0,1280,960); // TODO hardcoded, doesn't seem to do anything in core profile

                glBindTexture(GL_TEXTURE_2D, tex_id);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                curr_entry += sizeof(render_entry_texture_t);
            } break;

            case RENDER_ENTRY_TYPE_RECT:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;

                // TODO opengl code here
                // ...

                curr_entry += sizeof(render_entry_rect_t);
            } break;

            case RENDER_ENTRY_TYPE_TEXTURE_MOD:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_texture_mod_t* mod = (render_entry_texture_mod_t*) curr_entry;

                // TODO opengl code here
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

                flush_batch(); // TODO find a better place to call this ?

                /* TODO opengl code here */
                // NOTE maybe this shouldn't be a renderer command after all and
                // instead the platform layer just calls this after processing
                // cmds is done
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
    cmds->entry_count = 0;
    cmds->buf_offset  = cmds->buf;
}

// stubs
void renderer_destroy(renderer_t* renderer) {}
