#include "menulayer.h"
#include "platform.h"
#include "resourcemgr.h"
#include "event.h"
#include "globals.h"
#include "utils.h"

struct Button
{
    const char* label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    rect_t     box;
    // TODO maybe use 1 tex w/ an array of rects
    texture_t* tex[COUNT];
    texture_t* text_texture;
    rect_t     text_box;
    std::function<void(void)> callback;
};

static std::vector<Button> btns;
static texture_t* btn_inactive_tex;
static texture_t* btn_hover_tex;
static texture_t* btn_pressed_tex;
static texture_t* greyout_tex;
b32 g_layer_menu_is_active = true;

// create & get needed texs + create buttons & texs for labels
void layer_menu_init()
{
    g_layer_menu_is_active = false;

    // TODO use a resourcemgr or similar
    btn_inactive_tex = resourcemgr_texture_load("res/button.png");
    btn_hover_tex    = resourcemgr_texture_load("res/button_hover.png");
    btn_pressed_tex  = resourcemgr_texture_load("res/button_pressed.png");

    Button b1 = { .label = "CONTINUE", .state = Button::NONE, .box = {800, 475,300,100},
                  .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
    Button b2 = { .label = "OPTIONS",  .state = Button::NONE, .box = {800, 600,300,100},
                  .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
    Button b3 = { .label = "EXIT",     .state = Button::NONE, .box = {800, 725,300,100},
                  .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };

    // ADD CALLBACKS
    b1.callback = [&]() { g_layer_menu_is_active = false; };
    b2.callback = []()
    {
        /*printf("Trying to set VSYNC. Set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC))*/;
        /*SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);*/
        /*printf("Now set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC));*/
        printf("Options button pressed\n");
    };
    b3.callback = []()  { exit(1); };

    btns.push_back(b1);
    btns.push_back(b2);
    btns.push_back(b3);

    // for greying out game in the background when paused
    greyout_tex = resourcemgr_texture_load("res/greyout.png");

    // ADD TEXT TO BUTTONS
    platform_font_init();
    font_t* btn_font   = resourcemgr_font_load("res/ubuntumono.ttf");
    color_t text_color = {200,200,200,230};
    for (auto& b : btns)
    {
        surface_t* text_surf = platform_text_render(btn_font, b.label, text_color, 400);
        b.text_texture = platform_texture_create_from_surface(globals.window, text_surf);
        platform_texture_query(b.text_texture, NULL, NULL, &b.text_box.w, &b.text_box.h);
        platform_surface_destroy(text_surf);
    }
    // ResourceManager<TTF_Font*>::free("res/ubuntumono.ttf");
}

void layer_menu_handle_event()
{
    point_t mouse = {globals.game_input.mouse.pos.x, globals.game_input.mouse.pos.y};

    for (auto& b : btns)
    {
        if (point_in_rect(mouse, b.box)) b.state = Button::HOVER;
        else b.state = Button::NONE;
    }

    if (input_pressed(globals.game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
    {
        for (auto& b : btns)
        {
            if (b.state == Button::HOVER)
            {
                //evn->handled = true;
                b.state = Button::PRESSED;
                b.callback();
            }
        }
    }
}

void layer_menu_render()
{
    // grey out background
    platform_render_texture(globals.window, greyout_tex, NULL, NULL);

    // NOTE framerate-dependant
    // make buttons alpha 'pulsate'
    static i8 incr = 1;
    static u8 alpha = 200;
    alpha += incr;
    if (alpha > 254) incr = -1;
    if (alpha < 150)  incr = 1;

    // TODO dont call sdl render functions ourselves
    for (auto& b : btns)
    {
        if (b.state == Button::HOVER)
        {
            platform_texture_set_blend_mode(b.tex[b.state], 1 /*SDL_BLENDMODE_BLEND*/);
            platform_texture_set_alpha_mod(b.tex[b.state], alpha);
            //SDL_SetTextureColorMod(b.tex[b.state],100,100,4);
        }

        platform_render_texture(globals.window, b.tex[b.state], NULL, &b.box);
        rect_t text_dst = { b.box.x + b.box.w/2 - b.text_box.w/2,
                           b.box.y + b.box.h/2 - b.text_box.h/2,
                           b.text_box.w, b.text_box.h};
        platform_render_texture(globals.window, b.text_texture, NULL, &text_dst);

    }
}
