#include "imguilayer.h"
//#include "renderwindow.h"
#include "event.h"
#include "globals.h"

void layer_imgui_init()
{
#ifdef IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiSDL::Initialize(globals.rw->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
    // don't call an init function, but all of them require a rendering api
    // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
    // eventhandling. We expose the internal function below to circumvent that.
    ImGui_ImplSDL2_Init(globals.rw->window);
    ImGui::StyleColorsDark();
#endif
}

void layer_imgui_destroy()
{
#ifdef IMGUI
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
#endif
}

void layer_imgui_handle_event(Event& e)
{
#ifdef IMGUI
    ImGui_ImplSDL2_ProcessEvent(&e.sdl);
    ImGuiIO& io = ImGui::GetIO();
    e.handled = io.WantCaptureMouse ;//|| io.WantCaptureKeyboard;
    //e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
    //e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
#endif
}

void layer_imgui_begin()
{
#ifdef IMGUI
    ImGui_ImplSDL2_NewFrame(globals.rw->window);
    ImGui::NewFrame();
#endif
}

void layer_imgui_end()
{
#ifdef IMGUI
    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());
#endif
}
