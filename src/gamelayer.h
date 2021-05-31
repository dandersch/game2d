#pragma once

#include "pch.h"
#include "layer.h"
#include "collision.h"
#include "player.h"
#include "camera.h"
#include "entitymgr.h"

const int MAX_RENDER_LAYERS = 100;

class GameLayer : public Layer
{
public:
     GameLayer() : Layer() {}
    ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event& event) override;
    virtual void OnUpdate(f32 dt) override;
    virtual void OnRender() override;
    virtual void OnImGuiRender() override;

public:
    Camera cam;

private:
    bool debugDraw = false;
    Entity* focusedEntity = nullptr;
    SDL_Rect focusArrow = {64,32,16,32}; // TODO hardcoded
};
