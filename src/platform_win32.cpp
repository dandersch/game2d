#include "base.h"
#include "platform.h"

#ifdef PLATFORM_WIN32
#include <windows.h>
#include <tchar.h>

extern int game_main();

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (iMsg)
           {
           case WM_PAINT :
                hdc = BeginPaint (hwnd, &ps);

                GetClientRect (hwnd, &rect);
                SetBkMode(hdc, TRANSPARENT);

                DrawText (hdc, "Hello World!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
                EndPaint (hwnd, &ps);
                return 0;

           case WM_DESTROY :
                PostQuitMessage (0);
                return 0;
           }

    return DefWindowProc (hwnd, iMsg, wParam, lParam);
}

struct platform_window_t
{
    HWND handle;
};
struct windows_t
{
    HINSTANCE hInst;
    HINSTANCE hInstPrev;
    PSTR cmdline;
    int cmdshow;
};
static windows_t win_main_args  = {0};
static platform_window_t window = {0};

int APIENTRY WinMain(HINSTANCE hInst,     // handle (h) for this program
                     HINSTANCE hInstPrev, // our parent program
                     PSTR cmdline,        // (L)PSTR = (long) pointer string
                     int cmdshow)         // ...
{
    win_main_args = { hInst, hInstPrev, cmdline, cmdshow };
    game_main();
    return 0;
}

platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX wndclass;

    wndclass.cbSize        = sizeof(wndclass);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = win_main_args.hInst;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = title;
    wndclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx (&wndclass);

    hwnd = CreateWindow (title,               // window class name
                         "Hello World",       // window caption
                         WS_OVERLAPPEDWINDOW, // window style
                         CW_USEDEFAULT,       // initial x position
                         CW_USEDEFAULT,       // initial y position
                         screen_width, //CW_USEDEFAULT,       // initial x size
                         screen_height, //CW_USEDEFAULT,       // initial y size
                         NULL,                // parent window handle
                         NULL,                // window menu handle
                         win_main_args.hInst, // program instance handle
                         NULL);               // creation parameters

    window.handle = hwnd;

    ShowWindow (hwnd, win_main_args.cmdshow);
    UpdateWindow (hwnd);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }

    return &window;
}

void platform_window_close(platform_window_t* window)
{
}

// EVENTS //////////////////////////////////////////////////////////////////////////////////////////
void platform_event_loop(game_input_t* input)
{
}

// TODO replace all calls to this with calls to platform_render_texture
void platform_render_sprite(platform_window_t* window, const sprite_t& spr,
                            v3f position, f32 scale, u32 flip_type)
{
}

void platform_render_texture(platform_window_t* window, texture_t* texture, rect_t* src, rect_t* dst)
{
}

void platform_render_clear(platform_window_t* window)
{
}

void platform_render_present(platform_window_t* window)
{
}

void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a)
{
}

void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos)
{
}

void platform_debug_draw_rect(platform_window_t* window, rect_t* dst)
{
}

texture_t* platform_texture_create_from_surface(platform_window_t* window, surface_t* surface)
{
}

texture_t* platform_texture_load(platform_window_t* window, const char* filename)
{
    return nullptr;
}

i32 platform_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    return 0;
}

i32 platform_texture_set_blend_mode(texture_t* tex, u32 mode)
{
    return 0;
}

i32 platform_texture_set_alpha_mod(texture_t* tex, u8 alpha)
{
    return 0;
}

void platform_surface_destroy(surface_t* surface)
{
}

// SDL TTF extension ///////////////////////////////////////////////////////////////////////////////
font_t* platform_font_load(const char* filename, i32 ptsize)
{
    return nullptr;
}

void platform_font_init()
{
}

// TODO pass options to render blended/wrapped
surface_t* platform_text_render(font_t* font, const char* text, color_t color, u32 wrap_len)
{
    return nullptr;
}

u32  platform_ticks() { return 0; }

void platform_quit()
{
}

// IMGUI BACKEND ///////////////////////////////////////////////////////////////////////////////////
void platform_imgui_init(platform_window_t* window, u32 screen_width, u32 screen_height)
{
#ifdef IMGUI
#endif
}

void platform_imgui_destroy()
{
#ifdef IMGUI
#endif
}

void platform_imgui_event_handle(game_input_t* input)
{
#ifdef IMGUI
#endif
}

void platform_imgui_begin(platform_window_t* window)
{
#ifdef IMGUI
#endif
}

void platform_imgui_end()
{
#ifdef IMGUI
#endif
}
#endif // PLATFORM_WIN32
