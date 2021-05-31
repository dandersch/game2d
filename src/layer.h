#pragma once

#include "pch.h"

struct Event
{
    SDL_Event sdl;
    b32 handled = false;
};

class Layer
{
public:
    Layer() {}
    virtual ~Layer() = default;

    virtual void OnAttach() {} // init stuff goes in here
    virtual void OnDetach() {} // cleanup stuff goes in here
    virtual void OnUpdate(f32 dt) {}
    virtual void OnRender() {}
    virtual void OnImGuiRender() {}
    virtual void OnEvent(Event& event) {}

    b32 active = true;
};
