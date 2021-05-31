#pragma once

#ifdef IMGUI

#include "pch.h"
#include "layer.h"
#include "renderwindow.h"

class ImGuiLayer : public Layer
{
public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(Event& e) override;
    void Begin();
    void End();
};

#endif
