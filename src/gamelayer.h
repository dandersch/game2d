#pragma once

#include "pch.h"
#include "layer.h"
#include "collision.h"
#include "player.h"
#include "camera.h"

// NOTE emscript build fails when entity array is too large...
// currently this happens at around 20k
#define MAX_ENTITIES  15000
const int MAX_RENDER_LAYERS = 100;

class GameLayer : public Layer
{
public:
     GameLayer() : Layer("GameLayer") {}
    ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event& event) override;
    virtual void OnUpdate(f32 dt) override;
    virtual void OnRender() override;
    virtual void OnImGuiRender() override;

public:
    SDL_Texture* txtTex;
    Camera cam;

private:
    // compile times blow up when this is not static and MAX_ENTITIES is large
    static Entity ents[MAX_ENTITIES];
    bool debugDraw = false;
};
