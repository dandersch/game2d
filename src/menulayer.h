#pragma once

#include "SDL_render.h"
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
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event& event) override;
    virtual void OnUpdate(f32 dt) override;
    virtual void OnRender() override;

private:
    std::vector<Button> btns;
    SDL_Texture* btn_inactive_tex;
    SDL_Texture* btn_hover_tex;
    SDL_Texture* btn_pressed_tex;
    SDL_Texture* greyout_tex;
};
