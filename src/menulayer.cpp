#include "menulayer.h"
#include "resourcemgr.h"
#include "event.h"
#include "globals.h"

/*
struct Button
{
    std::string label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    SDL_Rect     box;
    // TODO maybe use 1 tex w/ an array of sdl_rects
    SDL_Texture* tex[COUNT];
    SDL_Texture* txtTex;
    rect_t       txtBox;
    std::function<void(void)> callback;
};

static std::vector<Button> btns;
static SDL_Texture* btn_inactive_tex;
static SDL_Texture* btn_hover_tex;
static SDL_Texture* btn_pressed_tex;
static SDL_Texture* greyout_tex;
b32 g_layer_menu_is_active = true;

// create & get needed texs + create buttons & texs for labels
void layer_menu_init()
{
    g_layer_menu_is_active = false;

    // TODO use a resourcemgr or similar
    btn_inactive_tex = ResourceManager<SDL_Texture*>::get("res/button.png");
    btn_hover_tex    = ResourceManager<SDL_Texture*>::get("res/button_hover.png");
    btn_pressed_tex  = ResourceManager<SDL_Texture*>::get("res/button_pressed.png");

    Button b1 = { .label = "CONTINUE", .state = Button::NONE, .box = {800, 475,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
    Button b2 = { .label = "OPTIONS",  .state = Button::NONE, .box = {800, 600,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
    Button b3 = { .label = "EXIT",     .state = Button::NONE, .box = {800, 725,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };

    // ADD CALLBACKS
    b1.callback = [&]() { g_layer_menu_is_active = false; };
    b2.callback = []()  { printf("Trying to set VSYNC. Set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC));
                          SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);
                          printf("Now set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC));};
    b3.callback = []()  { exit(1); };

    btns.push_back(b1);
    btns.push_back(b2);
    btns.push_back(b3);

    // for greying out game in the background when paused
    greyout_tex = ResourceManager<SDL_Texture*>::get("res/greyout.png");

    // ADD TEXT TO BUTTONS
    TTF_Init();
    TTF_Font* btnFont = ResourceManager<TTF_Font*>::get("res/ubuntumono.ttf");
    SDL_Color textColor   = {200,200,200,230};
    for (auto& b : btns)
    {
        SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(btnFont, b.label.c_str(), textColor, 400);
        SDL_ERROR(textSurf);
        b.txtTex = SDL_CreateTextureFromSurface(globals.rw->renderer, textSurf);
        SDL_ERROR(b.txtTex);
        SDL_QueryTexture(b.txtTex, NULL, NULL, &b.txtBox.w, &b.txtBox.h);
        SDL_FreeSurface(textSurf);
    }
    // ResourceManager<TTF_Font*>::free("res/ubuntumono.ttf");
}

void layer_menu_handle_event(Event& event)
{
    SDL_Event evn = event.sdl;
    SDL_Point mouse = { evn.motion.x, evn.motion.y };

    switch (evn.type) {
    case SDL_KEYDOWN:
        switch (evn.key.keysym.sym)
        {
            case SDLK_UP: break;
            case SDLK_DOWN: break;
        }
        break;
    case SDL_MOUSEMOTION:
        for (auto& b : btns)
        {
            if (SDL_PointInRect(&mouse, &b.box))
                b.state = Button::HOVER;
            else
                b.state = Button::NONE;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        for (auto& b : btns)
        {
            if (b.state == Button::HOVER)
            {
                event.handled = true;
                b.state = Button::PRESSED;
                b.callback();
            }
        }
        break;
        break;
    }
}

void layer_menu_render()
{
    // grey out background
    SDL_RenderCopy(globals.rw->renderer, greyout_tex, NULL, NULL);

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
            SDL_SetTextureBlendMode(b.tex[b.state], SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(b.tex[b.state],alpha);
            //SDL_SetTextureColorMod(b.tex[b.state],100,100,4);
        }

        SDL_RenderCopy(globals.rw->renderer, b.tex[b.state], NULL, &b.box);
        SDL_Rect textDst = { b.box.x + b.box.w/2 - b.txtBox.w/2,
                             b.box.y + b.box.h/2 - b.txtBox.h/2,
                             b.txtBox.w, b.txtBox.h};
        SDL_RenderCopy(globals.rw->renderer, b.txtTex, NULL, &textDst);
    }
}

*/
