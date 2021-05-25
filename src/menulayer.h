#pragma once

#include "SDL_image.h"
#include "SDL_rect.h"
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
    // TODO callback
};

class MenuLayer : public Layer
{
public:
    // create & get needed texs + create buttons & texs for labels
    virtual void OnAttach()
    {
        // TODO use a resourcemgr or similar
        btn_inactive_tex = IMG_LoadTexture(rw->renderer, "res/button.png");
        btn_hover_tex    = IMG_LoadTexture(rw->renderer, "res/button_hover.png");
        btn_pressed_tex  = IMG_LoadTexture(rw->renderer, "res/button_pressed.png");

        Button b1 = { .label = "CLICK1", .state = Button::NONE, .box = {800, 475,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
        Button b2 = { .label = "CLICK2", .state = Button::NONE, .box = {800, 600,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
        Button b3 = { .label = "CLICK3", .state = Button::NONE, .box = {800, 725,300,100}, .tex = { btn_inactive_tex, btn_hover_tex, btn_pressed_tex } };
        btns.push_back(b1);
        btns.push_back(b2);
        btns.push_back(b3);
    }

    virtual void OnDetach() {}

    virtual void OnEvent(Event& event)
    {
        SDL_Event evn = event.evn;
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
                }
            }
            break;
            break;
        }
    }

    virtual void OnUpdate(f32 dt) {}

    virtual void OnRender()
    {
        // TODO dont call sdl render functions ourselves
        for (auto& b : btns)
            SDL_RenderCopy(rw->renderer, b.tex[b.state], NULL, &b.box);
    }

    std::vector<Button> btns;
    SDL_Texture* btn_inactive_tex;
    SDL_Texture* btn_hover_tex;
    SDL_Texture* btn_pressed_tex;
};
