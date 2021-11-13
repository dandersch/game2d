#if defined(PLATFORM_SDL)

// NOTE SDL headers come from pch
#include "base.h"
#include "platform.h"
#include "platform_renderer.h"
#include "input.h"

struct platform_window_t
{
    SDL_Window*   handle;
    renderer_t*   renderer;
    u32           width;
    u32           height;
};

#define SDL_ERROR(x) if (!x) { printf("SDL ERROR: %s\n", SDL_GetError()); }

// UNITY BUILD
#include "platform_renderer.cpp" // NOTE needs to above opengl/sdl implementation
#ifdef USE_OPENGL // NOTE we could compile the renderer as a dll in the future...
  #include "platform_renderer_opengl.cpp"
#else
  #include "platform_renderer_sdl.cpp"
#endif

// game functions
typedef void (*game_main_loop_fn)(game_state_t*, platform_api_t);
struct game_api_t
{
    game_main_loop_fn      main_loop;
    int                    id;
};
global_var game_api_t game;

extern platform_api_t platform_api;
global_var b32 game_running = true;

#include <dlfcn.h>    // for opening shared objects (needs to be linked with -ldl)
#include <sys/stat.h> // for checking if dll changed on disk (TODO does it work crossplatform?)
global_var void* dll_handle = nullptr;
#ifdef PLATFORM_WIN32
  const char* GAME_DLL = "./dep/libgame.dll";
#else
  const char* GAME_DLL = "./dep/libgame.so";
#endif

// NOTE sdl also offers functions for dll loading, which might be crossplatform
// SDL_UnloadObject(), SDL_LoadObject(), SDL_LoadFunction()
// TODO use a (custom ?) error function and not printf
b32 platform_load_code()
{
    // unload old dll
    if (dll_handle)
    {
        game.main_loop    = nullptr;
        game.id           = 0;

        if (dlclose(dll_handle) != 0) printf("FAILED TO CLOSE DLL\n");
        dll_handle = nullptr;
    }

    // See https://nullprogram.com/blog/2014/12/23/
    // "It’s critically important that dlclose() happens before dlopen(). On my
    // system, dlopen() looks only at the string it’s given, not the file behind
    // it. Even though the file has been replaced on the filesystem, dlopen()
    // will see that the string matches a library already opened and return a
    // pointer to the old library. (Is this a bug?)"

    // NOTE try opening until it works, otherwise we need to sleep() for a moment to avoid a crash
    while (dll_handle == nullptr)
    {
        dll_handle = dlopen(GAME_DLL, RTLD_NOW);
        if (dll_handle == nullptr) printf("OPENING GAME DLL FAILED. TRYING AGAIN.\n");
    }

    if (dll_handle == nullptr)
    {
        printf("OPENING LIBGAME.SO FAILED\n");
        return false;
    }

    game.main_loop    = (game_main_loop_fn)    dlsym(dll_handle, "game_main_loop");

    if (!game.main_loop)
    {
        printf("FINDING GAME_MAIN FAILED\n");
        return false;
    }

    return true;
}

#include "memory.h"
struct game_memory_t
{
    mem_arena_t total_arena;
      mem_arena_t platform_arena;
        // mem_arena_t render_arena;
      mem_arena_t game_arena;
        // mem_arena_t entity_arena;

    //ring_buffer_t temp_storage; // TODO
};
game_memory_t memory = {};

struct game_state_t;
game_state_t* game_state = nullptr;

// TODO define TOTAL_MEMORY_SIZE
#define GAME_MEMORY_SIZE 67108864 // 2^26
// TODO define PLATFORM_MEMORY_SIZE

// entry point
int main(int argc, char* args[])
{
    mem_arena_init(&memory.total_arena, GAME_MEMORY_SIZE + 6000012);
    mem_arena_nested_init(&memory.total_arena, &memory.platform_arena, 6000012); // TODO hardcoded
    mem_arena_nested_init(&memory.total_arena, &memory.game_arena, GAME_MEMORY_SIZE);

    game_state = (game_state_t*) mem_arena_alloc(&memory.game_arena, GAME_MEMORY_SIZE);
    memset(game_state, 0, GAME_MEMORY_SIZE);
    {
        // get the game.id (inode) so we don't perform a code reload when first entering the main loop
        struct stat attr;
        stat(GAME_DLL, &attr);
        game.id = attr.st_ino;
    }
    platform_load_code(); // initial loading of the game dll
    game_running = true;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(game_main_loop, -1, 1); // NOTE no code reloading, outdated
#else
    while (game_running)
    {
        game.main_loop(game_state, platform_api);

        // check if dll/so changed on disk
        // NOTE: should only happen in debug builds
        struct stat attr;
        if ((stat(GAME_DLL, &attr) == 0) && (game.id != attr.st_ino))
        {
            printf("Attempting code reload\n");
            platform_load_code();
            game.id = attr.st_ino;
        }
    }

    // we can register that the game was quit in an 'invalid' way here, i.e. not
    // by e.g. pressing the quit button in the menu, but by forcing the window to close etc.
    exit(1);
#endif
}


platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height)
{
    if (SDL_Init(SDL_INIT_TIMER
                 | SDL_INIT_AUDIO
                 | SDL_INIT_VIDEO
                 // | SDL_INIT_JOYSTICK
                 // | SDL_INIT_HAPTIC
                 // | SDL_INIT_GAMECONTROLLER
                 // | SDL_INIT_EVENTS
                 // | SDL_INIT_SENSOR
                 // | SDL_INIT_NOPARACHUTE
                 // | SDL_INIT_EVERYTHING
                 ) != 0)
    {
        printf("SDL init failed: %s\n", SDL_GetError());
    }

    u32 window_flags = 0;

#ifdef USE_OPENGL // TODO is there a way to do this w/o ifdefs and w/o bringing SDL code into the renderer
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // NOTE doesn't seem to force 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // NOTE doesn't seem to force 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // turn on double buffering set the depth buffer to 24 bits
    // you may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window_flags |= SDL_WINDOW_OPENGL;
    window_flags |= SDL_WINDOW_RESIZABLE;

    /*
     SDL can load OpenGL function pointers with
          int SDL_GL_LoadLibrary(const char* path);
          void* SDL_GL_GetProcAddress(const char* proc);
     also see
          https://wiki.libsdl.org/SDL_GL_GetProcAddress
    */
#else
    // ...
#endif

    platform_window_t* window = (platform_window_t*) mem_arena_alloc(&memory.platform_arena,
                                                                     sizeof(platform_window_t));
    window->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      screen_width, screen_height, window_flags);
    SDL_ERROR(window->handle);
    window->width  = screen_width;
    window->height = screen_height;

#ifdef USE_OPENGL
    window->renderer = (renderer_t*) mem_arena_alloc(&memory.platform_arena, sizeof(renderer_t));
    SDL_GLContext gl_context = SDL_GL_CreateContext(window->handle);
    SDL_ERROR(gl_context); // Failed to create OpenGL context.
    SDL_GL_MakeCurrent(window->handle, gl_context);
    window->renderer->gl_context = gl_context;
#endif

    renderer_init(window, &memory.platform_arena);

    SDL_version version;
    SDL_GetVersion(&version);
    printf("SDL VERSION: %u, %u, %u\n", version.major, version.minor, version.patch);

    return window;
}


void platform_window_close(platform_window_t* window)
{
    renderer_destroy(window->renderer);
    SDL_DestroyWindow(window->handle);
}


// should return a file_t w/ handle & buffer
file_t platform_file_load(const char* file_name)
{
    file_t file;
    // NOTE loading in file in binary-mode. Will this cause problems on
    // other platforms w/ different newline characters?
    SDL_RWops* sdl_file = SDL_RWFromFile(file_name, "r+b");
    if(!sdl_file) printf("Unable to open file! SDL Error: %s\n", SDL_GetError());

    file.size    = sdl_file->size(sdl_file);
    u8* file_buf = (u8*) malloc(file.size);
    SDL_RWread(sdl_file, file_buf, sizeof(u8), file.size);

    file.handle = sdl_file;
    file.buffer = file_buf;

    return file;
}


void platform_file_close(file_t file)
{
    i32 success = SDL_RWclose((SDL_RWops*) file.handle);
    free(file.buffer);
    SDL_ERROR(!success);
}


b32 platform_file_save(u8* file_name, u8* buffer)
{
    UNREACHABLE("function not implementd");
    return false;
}


// EVENTS //////////////////////////////////////////////////////////////////////////////////////////

// counts up button presses between last and next frame
// TODO check if this actually works, i.e. if you can press a button more than once between frames
internal_fn inline
void input_event_process(game_input_state_t* new_state, b32 is_down)
{
    if(new_state->is_down != is_down)
    {
        new_state->is_down = is_down;
        ++new_state->up_down_count;
    }
}


void platform_event_loop(game_input_t* input, platform_window_t* window)
{
    // TODO remove hardcoded resetting of halftransitioncount
    for(int key_idx = 0; key_idx < 128; /* TODO hardcoded */ ++key_idx)
        input->keyboard.keys[key_idx].up_down_count = 0;
    for(int btn_idx = 0; btn_idx < MOUSE_BUTTON_COUNT; /* TODO hardcoded */ ++btn_idx)
        input->mouse.buttons[btn_idx].up_down_count = 0;
    for(int f_idx = 0; f_idx < 13; /* TODO hardcoded */ ++f_idx)
        input->keyboard.f_key_pressed[f_idx] = false;
    input->keyboard.key_up.up_down_count = 0;
    input->keyboard.key_down.up_down_count = 0;
    input->keyboard.key_left.up_down_count = 0;
    input->keyboard.key_right.up_down_count = 0;
    input->mouse.wheel = 0;

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) // TODO use SDL_WaitEvent ?
    {
        switch (sdl_event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                SDL_Keycode keycode = sdl_event.key.keysym.sym;
                b32 is_down         = (sdl_event.key.state == SDL_PRESSED);

                if(sdl_event.key.repeat == 0)
                {
                    if (is_down)
                    {
                        if((keycode >= SDLK_F1) && (keycode <= SDLK_F12))
                        {
                            input->keyboard.f_key_pressed[keycode - SDLK_F1 + 1] = true;
                        }
                    }

                    if (keycode == SDLK_UP)    input_event_process(&input->keyboard.key_up, is_down);
                    if (keycode == SDLK_DOWN)  input_event_process(&input->keyboard.key_down, is_down);
                    if (keycode == SDLK_LEFT)  input_event_process(&input->keyboard.key_left, is_down);
                    if (keycode == SDLK_RIGHT) input_event_process(&input->keyboard.key_right, is_down);

                    // NOTE SDL Keycodes (SDLK_*) seem to map to ascii for a-z
                    // but other characters (e.g. winkey/f-keys) go over 128,
                    // so we mask off bits here
                    keycode &= 255;
                    input_event_process(&input->keyboard.keys[keycode], is_down);
                }

            } break;

            case SDL_MOUSEMOTION:
            {
                input->mouse.pos = {sdl_event.motion.x, sdl_event.motion.y, 0};
            } break;

            case SDL_MOUSEWHEEL:
            {
                input->mouse.wheel = sdl_event.wheel.y;
            } break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                auto button = sdl_event.button.button;
                b32 is_down = sdl_event.button.state;
                if (button == SDL_BUTTON_LEFT)
                    input_event_process(&input->mouse.buttons[MOUSE_BUTTON_LEFT], is_down);
            } break;

            case SDL_QUIT: { game_running = false; } break;

            case SDL_WINDOWEVENT:
            {
                switch (sdl_event.window.event) // TODO why do we need to switch on event and not on type?
                {
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        game_running = false;
                    } break;

                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        // TODO duplicated data
                        input->window_width  = sdl_event.window.data1;
                        input->window_height = sdl_event.window.data2;
                        window->width        = sdl_event.window.data1;
                        window->height       = sdl_event.window.data2;
                    } break;

                    case SDL_WINDOWEVENT_HIDDEN:         /**< Window has been hidden */
                    case SDL_WINDOWEVENT_ENTER:          /**< Window has gained mouse focus */
                    case SDL_WINDOWEVENT_LEAVE:          /**< Window has lost mouse focus */
                    case SDL_WINDOWEVENT_FOCUS_GAINED:   /**< Window has gained keyboard focus */
                    case SDL_WINDOWEVENT_FOCUS_LOST:     /**< Window has lost keyboard focus */
                    {
                        // TODO
                    } break;
                }
            } break;
        }
    }
}


void platform_render(platform_window_t* window)
{
    renderer_cmd_buf_process(window); // TODO this draws over imgui
}

// only draws colored rects (colliders) for now
// TODO move into renderer
void platform_debug_draw(platform_window_t* window, rect_t collider_box, v3f pos, color_t color, u32 scale)
{
    rect_t dst = {(int) pos.x + collider_box.left, (int) pos.y + collider_box.top,
                  (i32) (scale * collider_box.w), (i32) (scale * collider_box.h)};

    // don't draw 'empty' colliders (otherwise it will draw points & lines)
    // TODO maybe put this into the renderer
    if (!SDL_RectEmpty((SDL_Rect*) &dst)) // == if (!(dst.h <= 0.f && dst.w <= 0.f))
    {
        //renderer_push_rect({dst, color});
    }

    //SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
    // TODO isn't where it's expected
    // Draw pivot point
    // SDL_RenderDrawPointF(window->renderer, pos.x, pos.y);
}


u64 platform_debug_performance_counter()
{
    return SDL_GetPerformanceCounter();
}


void platform_surface_destroy(surface_t* surface)
{
    SDL_FreeSurface((SDL_Surface*) surface);
}


// SDL TTF extension ///////////////////////////////////////////////////////////////////////////////
font_t* platform_font_load(const char* filename, i32 ptsize)
{
    TTF_Font* font = TTF_OpenFont(filename, ptsize);
    SDL_ERROR(font);
    return font;
}


void platform_font_init()
{
    TTF_Init();
}


// TODO pass options to render blended/wrapped
// NOTE this is not (as of now) an SDL Renderer specific function
surface_t* platform_text_render(font_t* font, const char* text, color_t color, u32 wrap_len)
{
    SDL_Surface* text_surf = TTF_RenderText_Blended_Wrapped((TTF_Font*) font, text,
                                                            *((SDL_Color*) &color), wrap_len);
    SDL_ERROR(text_surf);
    return text_surf;
}


u32  platform_ticks() { return SDL_GetTicks(); }


void platform_quit()
{
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


platform_api_t platform_api =
{
  &platform_file_load,
  &platform_file_save,
  &platform_file_close,
  &platform_window_open,
  &platform_window_close,
  &platform_event_loop,
  &platform_ticks,
  &platform_quit,
  &platform_render,
  &platform_surface_destroy,
  &platform_font_init,
  &platform_font_load,
  &platform_text_render,
  &platform_debug_draw,
  &platform_debug_performance_counter,
  { // renderer api
    &renderer_push_sprite,
    &renderer_push_texture,
    &renderer_push_texture_mod,
    &renderer_push_rect,
    &renderer_push_rect_outline,
    &renderer_push_clear,
    &renderer_push_present,

    &renderer_load_texture,
    &renderer_create_texture_from_surface,
    &renderer_texture_query,

    &renderer_upload_camera,
  },
};

#endif // PLATFORM_SDL
