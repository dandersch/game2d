#pragma once

#include "pch.h"

struct Event
{
    SDL_Event evn;
    b32 handled;
};

class Layer
{
public:
    Layer(const std::string& name = "Layer") : m_DebugName(name) {}
    virtual ~Layer() = default;

    virtual void OnAttach() {} // init stuff goes in here
    virtual void OnDetach() {} // cleanup stuff goes in here
    virtual void OnUpdate(f32 dt) {}
    virtual void OnRender() {}
    virtual void OnImGuiRender() {}
    virtual void OnEvent(Event& event) {}

    const std::string& GetName() const { return m_DebugName; }
protected:
    std::string m_DebugName;
};
