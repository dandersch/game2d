#include "menulayer.h"
#include "resourcemgr.h"
#include "globals.h"
#include "utils.h"

//static std::vector<Button> btns;
//static texture_t* btn_inactive_tex;
//static texture_t* btn_hover_tex;
//static texture_t* btn_pressed_tex;
//static texture_t* greyout_tex;
//b32 g_layer_menu_is_active = true;

#include "memory.h"
extern game_state_t* state;
// create & get needed texs + create buttons & texs for labels
void layer_menu_init()
{
    state->g_layer_menu_is_active = false;

    // TODO use a resourcemgr or similar
    state->btn_inactive_tex = resourcemgr_texture_load("res/button.png", state);
    state->btn_hover_tex    = resourcemgr_texture_load("res/button_hover.png", state);
    state->btn_pressed_tex  = resourcemgr_texture_load("res/button_pressed.png", state);

    Button b1 = { .label = "CONTINUE", .state = Button::NONE, .box = {800, 475,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };
    Button b2 = { .label = "OPTIONS",  .state = Button::NONE, .box = {800, 600,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };
    Button b3 = { .label = "EXIT",     .state = Button::NONE, .box = {800, 725,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };

    // ADD CALLBACKS
    b1.callback = [&]() { state->g_layer_menu_is_active = false; };
    b2.callback = []()
    {
        /*printf("Trying to set VSYNC. Set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC))*/;
        /*SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);*/
        /*printf("Now set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC));*/
        printf("Options button pressed!\n");
    };
    b3.callback = [&]()  { state->game_running = false; }; // TODO signal to platform layer

    state->btns.push_back(b1);
    state->btns.push_back(b2);
    state->btns.push_back(b3);

    // for greying out game in the background when paused
    state->greyout_tex = resourcemgr_texture_load("res/greyout.png", state);

    // ADD TEXT TO BUTTONS
    platform.font_init();
    font_t* btn_font   = resourcemgr_font_load("res/ubuntumono.ttf", state);
    color_t text_color = {200,200,200,230};
    for (auto& b : state->btns)
    {
        surface_t* text_surf = platform.text_render(btn_font, b.label, text_color, 400);
        b.text_texture = platform.texture_create_from_surface(state->window, text_surf);
        platform.texture_query(b.text_texture, NULL, NULL, &b.text_box.w, &b.text_box.h);
        platform.surface_destroy(text_surf);
    }
    // ResourceManager<TTF_Font*>::free("res/ubuntumono.ttf");
}

void layer_menu_handle_event()
{
    point_t mouse = {state->game_input.mouse.pos.x, state->game_input.mouse.pos.y};

    for (auto& b : state->btns)
    {
        if (point_in_rect(mouse, b.box)) b.state = Button::HOVER;
        else b.state = Button::NONE;
    }

    if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
    {
        for (auto& b : state->btns)
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
    platform.render_texture(state->window, state->greyout_tex, NULL, NULL);

    // NOTE framerate-dependant
    // make buttons alpha 'pulsate'
    local i8 incr = 1;
    local u8 alpha = 200;
    alpha += incr;
    if (alpha > 254) incr = -1;
    if (alpha < 150)  incr = 1;

    // TODO dont call sdl render functions ourselves
    for (auto& b : state->btns)
    {
        if (b.state == Button::HOVER)
        {
            platform.texture_set_blend_mode(b.tex[b.state], 1 /*SDL_BLENDMODE_BLEND*/);
            platform.texture_set_alpha_mod(b.tex[b.state], alpha);
            //SDL_SetTextureColorMod(b.tex[b.state],100,100,4);
        }

        platform.render_texture(state->window, b.tex[b.state], NULL, &b.box);
        rect_t text_dst = { b.box.x + b.box.w/2 - b.text_box.w/2,
                           b.box.y + b.box.h/2 - b.text_box.h/2,
                           b.text_box.w, b.text_box.h};
        platform.render_texture(state->window, b.text_texture, NULL, &text_dst);

    }
}
