#pragma once

#ifdef IMGUI
#include "SDL_events.h"
#include "pch.h"
#include "layer.h"
#include "renderwindow.h"

class ImGuiLayer : public Layer
{
public:
     ImGuiLayer() : Layer("ImGuiLayer") {}
    ~ImGuiLayer() = default;

    virtual void OnAttach() override
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiSDL::Initialize(rw->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
        // don't call an init function, but all of them require a rendering api
        // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
        // eventhandling. We expose the internal function below to circumvent that.
        ImGui_ImplSDL2_Init(rw->window);
        ImGui::StyleColorsDark();
    }

    virtual void OnDetach() override
    {
        ImGuiSDL::Deinitialize();
        ImGui::DestroyContext();
    }

    virtual void OnEvent(Event& e) override
    {
        ImGui_ImplSDL2_ProcessEvent(&e.sdl);
        ImGuiIO& io = ImGui::GetIO();
        e.handled = io.WantCaptureMouse || io.WantCaptureKeyboard;
        //e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
        //e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }

    void Begin()
    {
        ImGui_ImplSDL2_NewFrame(rw->window);
        ImGui::NewFrame();
    }

    void End()
    {
        ImGui::Render();
        ImGuiSDL::Render(ImGui::GetDrawData());
    }

    // bool m_BlockEvents = true;
};
#endif
