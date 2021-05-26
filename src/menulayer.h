#pragma once

#include "pch.h"
#include "layer.h"
#include "resourcemgr.h"

struct Button
{
    std::string label; // TODO font to render
    enum State {NONE, HOVER, PRESSED, COUNT} state;
    SDL_Rect     box;
    // TODO maybe use 1 tex w/ an array of sdl_rects
    SDL_Texture* tex[COUNT];
    SDL_Texture* txtTex;
    SDL_Rect     txtBox;
    std::function<void(void)> callback;
};

class MenuLayer : public Layer
{
public:
    // create & get needed texs + create buttons & texs for labels
    virtual void OnAttach()
    {
        // TODO use a resourcemgr or similar
        btn_inactive_tex = ResourceManager<SDL_Texture*>::get("res/button.png");
        btn_hover_tex    = ResourceManager<SDL_Texture*>::get("res/button_hover.png");
        btn_pressed_tex  = ResourceManager<SDL_Texture*>::get("res/button_pressed.png");

        Button b1 = { .label = "CONTINUE", .state = Button::NONE, .box = {800, 475,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
        Button b2 = { .label = "OPTIONS",  .state = Button::NONE, .box = {800, 600,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
        Button b3 = { .label = "EXIT",     .state = Button::NONE, .box = {800, 725,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };

        // ADD CALLBACKS
        b1.callback = [&]() { active = false; };
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
            b.txtTex = SDL_CreateTextureFromSurface(rw->renderer, textSurf);
            SDL_ERROR(b.txtTex);
            SDL_QueryTexture(b.txtTex, NULL, NULL, &b.txtBox.w, &b.txtBox.h);
            SDL_FreeSurface(textSurf);
        }
        // ResourceManager<TTF_Font*>::free("res/ubuntumono.ttf");
    }

    virtual void OnDetach() {}

    virtual void OnEvent(Event& event)
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

    virtual void OnUpdate(f32 dt) {}

    virtual void OnRender()
    {
        // grey out background
        SDL_RenderCopy(rw->renderer, greyout_tex, NULL, NULL);

        // TODO dont call sdl render functions ourselves
        for (auto& b : btns)
        {
            SDL_RenderCopy(rw->renderer, b.tex[b.state], NULL, &b.box);
            SDL_Rect textDst = { b.box.x + b.box.w/2 - b.txtBox.w/2,
                                 b.box.y + b.box.h/2 - b.txtBox.h/2,
                                 b.txtBox.w, b.txtBox.h};
            SDL_RenderCopy(rw->renderer, b.txtTex, NULL, &textDst);
        }
    }

    std::vector<Button> btns;
    SDL_Texture* btn_inactive_tex;
    SDL_Texture* btn_hover_tex;
    SDL_Texture* btn_pressed_tex;

    SDL_Texture* greyout_tex;
};
