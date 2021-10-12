#if defined(PLATFORM_SDL)

// NOTE SDL headers come from pch
#include "base.h"
#include "platform.h"
#include "platform_renderer.h"
#include "input.h"

struct platform_window_t
{
    SDL_Window*   handle;
    renderer_t*   renderer; // TODO needs to opaque/not renderer specific
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
typedef void (*game_main_loop_fn)();
typedef b32  (*game_init_fn)(game_state_t*);
typedef void (*game_state_update_fn)(game_state_t*, platform_api_t);
typedef b32  (*game_quit_fn)();
struct game_api_t
{
    game_state_update_fn   state_update;
    game_init_fn           init;
    game_main_loop_fn      main_loop;
    game_quit_fn           quit;
    int                    id;
};
static game_api_t game;

void platform_quit();
extern platform_api_t platform_api;
static b32 game_running = true;

#include <dlfcn.h>    // for opening shared objects (needs to be linked with -ldl)
#include <sys/stat.h> // for checking if dll changed on disk (TODO does it work crossplatform?)
static void* dll_handle = nullptr;
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
        game.state_update = nullptr;
        game.main_loop    = nullptr;
        game.init         = nullptr;
        game.quit         = nullptr;
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

    // TODO pass memory to new dll
    // ...

    game.state_update = (game_state_update_fn) dlsym(dll_handle, "game_state_update");
    game.main_loop    = (game_main_loop_fn)    dlsym(dll_handle, "game_main_loop");
    game.init         = (game_init_fn)         dlsym(dll_handle, "game_init");
    game.quit         = (game_quit_fn)         dlsym(dll_handle, "game_quit");

    if (!game.main_loop)
    {
        printf("FINDING GAME_MAIN FAILED\n");
        return false;
    }

    return true;
}

struct game_state_t;
game_state_t* game_state = nullptr;
#define GAME_MEMORY_SIZE 67108864 // 2^26
// entry point
int main(int argc, char* args[])
{
    //platform_init();
    game_state = (game_state_t*) malloc(GAME_MEMORY_SIZE);
    memset(game_state, 0, GAME_MEMORY_SIZE);
    {
        // get the game.id (inode) so we don't perform a code reload when first
        // entering the main loop
        struct stat attr;
        stat(GAME_DLL, &attr);
        game.id = attr.st_ino;
    }
    platform_load_code(); // initial loading of the game dll
    game.state_update(game_state, platform_api);
    game.init(game_state);
    game_running = true;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(game_main_loop, -1, 1); // NOTE no code reloading
#else
    while (game_running)
    {
        game.main_loop();

        // check if dll/so changed on disk
        // NOTE: should only happen in debug builds
        struct stat attr;
        if ((stat(GAME_DLL, &attr) == 0) && (game.id != attr.st_ino))
        {
            printf("Attempting code reload\n");
            platform_load_code();
            game.id = attr.st_ino;
            game.state_update(game_state, platform_api); // pass memory to game dll
        }
    }
#endif

    game.quit();
    platform_quit();
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // turn on double buffering set the depth buffer to 24 bits
    // you may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window_flags |= SDL_WINDOW_OPENGL;

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

    // TODO don't call malloc
    platform_window_t* window = (platform_window_t*) malloc(sizeof(platform_window_t));

    window->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      screen_width, screen_height, window_flags);
    SDL_ERROR(window->handle);

    renderer_init(window);

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
internal inline
void input_event_process(game_input_state_t* new_state, b32 is_down)
{
    if(new_state->is_down != is_down)
    {
        new_state->is_down = is_down;
        ++new_state->up_down_count;
    }
}

void platform_event_loop(game_input_t* input)
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

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
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

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                auto button = sdl_event.button.button;
                b32 is_down = sdl_event.button.state;
                if (button == SDL_BUTTON_LEFT)
                    input_event_process(&input->mouse.buttons[MOUSE_BUTTON_LEFT], is_down);
            } break;

            case SDL_QUIT: { /*input->quit_requested = true;*/ game_running = false; } break;
            case SDL_WINDOWEVENT:
            {
                if (sdl_event.window.type == SDL_WINDOWEVENT_CLOSE)
                {
                        //input->quit_requested = true;
                    game_running = false;
                }
            } break;
        }
    }
}

void platform_render_texture(platform_window_t* window, texture_t* texture, rect_t* src, rect_t* dst)
{
    rect_t copy_src = {0};
    rect_t copy_dst = {0};
    if (src) copy_src = *src; // TODO temp
    if (dst) copy_dst = *dst; // TODO temp

    render_entry_texture_t draw_tex = {texture, copy_src, copy_dst};
    renderer_push_texture(draw_tex);
}

// TODO replace all calls to this with calls to platform_render_texture
// NOTE we are not actually doing anything w/ flip_type
void platform_render_sprite(platform_window_t* window, texture_t* sprite_tex, rect_t sprite_box,
                            v3f position, f32 scale, u32 flip_type)
{
    //SDL_Rect dst = {(int) position.x, (int) position.y, (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};
    rect_t dst = {(int) position.x, (int) position.y, (i32) (scale * sprite_box.w), (i32) (scale * sprite_box.h)};

    // NOTE: we could flip the texture w/ SDL_RenderCopyEx
    //SDL_RenderCopy(window->renderer, (SDL_Texture*) sprite_tex, (SDL_Rect*) &sprite_box, &dst);

    platform_render_texture(window, sprite_tex, &sprite_box, &dst);
}

void platform_render_clear(platform_window_t* window) { renderer_push_clear({}); }

void platform_render_present(platform_window_t* window)
{
    renderer_push_present({});
    renderer_cmd_buf_process(window); // TODO this draws over imgui
}

// only draws colored rects (colliders) for now
void platform_debug_draw(platform_window_t* window, rect_t collider_box, v3f pos, color_t color, u32 scale)
{
    rect_t dst = {(int) pos.x + collider_box.x, (int) pos.y + collider_box.y,
                  (i32) (scale * collider_box.w), (i32) (scale * collider_box.h)};

    // don't draw 'empty' colliders (otherwise it will draw points & lines)
    // TODO maybe put this into the renderer
    if (!SDL_RectEmpty((SDL_Rect*) &dst)) // == if (!(dst.h <= 0.f && dst.w <= 0.f))
    {
        renderer_push_rect({dst, color});
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

// TODO move into platform_renderer_sdl.cpp
texture_t* platform_texture_create_from_surface(platform_window_t* window, surface_t* surface)
{
    SDL_Texture* tex = SDL_CreateTextureFromSurface((SDL_Renderer*) window->renderer, (SDL_Surface*) surface);
    SDL_ERROR(tex);
    return tex;
}

// TODO move into platform_renderer_sdl.cpp
texture_t* platform_texture_load(platform_window_t* window, const char* filename)
{
    SDL_Texture* tex = IMG_LoadTexture((SDL_Renderer*) window->renderer, filename);
    SDL_ERROR(tex);
    return tex;
}

// TODO move into platform_renderer_sdl.cpp
i32 platform_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    return SDL_QueryTexture((SDL_Texture*) tex, format, access, w, h);
}

// TODO move into platform_renderer_sdl.cpp
i32 platform_texture_set_blend_mode(texture_t* tex, u32 mode)
{
    return SDL_SetTextureBlendMode((SDL_Texture*) tex, (SDL_BlendMode) mode);
}

// TODO move into platform_renderer_sdl.cpp
i32 platform_texture_set_alpha_mod(texture_t* tex, u8 alpha)
{
    return SDL_SetTextureAlphaMod((SDL_Texture*) tex, alpha);
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

// IMGUI BACKEND ///////////////////////////////////////////////////////////////////////////////////
void platform_imgui_init(platform_window_t* window, u32 screen_width, u32 screen_height)
{
#ifdef IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiSDL::Initialize((SDL_Renderer*) window->renderer, screen_width, screen_height);
    // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
    // don't call an init function, but all of them require a rendering api
    // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
    // eventhandling. We expose the internal function below to circumvent that.
    ImGui_ImplSDL2_Init(window->handle);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
#endif
}

void platform_imgui_destroy()
{
#ifdef IMGUI
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
#endif
}

void platform_imgui_event_handle(game_input_t* input)
{
#ifdef IMGUI
    // NOTE for some reason we don't have to call this...
    //ImGui_ImplSDL2_ProcessEvent(&e.sdl);

    ImGuiIO& io = ImGui::GetIO();

    // don't let mouse clicks on imgui propagate through underlying layers
    if (io.WantCaptureMouse)
    {
        // TODO hack: zero out mouseclick data so layers underneath don't react to them
        memset(input->mouse.buttons, 0, sizeof(input->mouse.buttons));
    }

    //io.WantCaptureKeyboard;

    //e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
    //e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
#endif
}

void platform_imgui_begin(platform_window_t* window)
{
#ifdef IMGUI
    ImGui_ImplSDL2_NewFrame(window->handle);
    ImGui::NewFrame();
#endif
}

void platform_imgui_end()
{
#ifdef IMGUI
    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData()); // TODO make this renderer agnostic
#endif
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
  &platform_render_sprite,
  &platform_render_texture,
  &platform_render_clear,
  &platform_render_present,
  &platform_texture_create_from_surface,
  &platform_texture_load,
  &platform_texture_query,
  &platform_texture_set_blend_mode,
  &platform_texture_set_alpha_mod,
  &platform_surface_destroy,
  &platform_font_init,
  &platform_font_load,
  &platform_text_render,
  &platform_debug_draw,
  &platform_debug_performance_counter,
  &platform_imgui_init,
  &platform_imgui_destroy,
  &platform_imgui_event_handle,
  &platform_imgui_begin,
  &platform_imgui_end
};

#endif // PLATFORM_SDL
