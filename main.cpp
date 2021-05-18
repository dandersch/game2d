#include "pch.h"

#include "entity.h"
#include "player.h"
#include "layer.h"
#include "renderwindow.h"
#include "imguilayer.h"

#define MAX_ENTITIES  10000

//The dimensions of the level
const int LEVEL_WIDTH  = 12800;
const int LEVEL_HEIGHT = 9600;

const int MAX_RENDER_LAYERS = 100;
const f32 TIME_PER_FRAME = (f32) 1/60;

SDL_Texture* tex;
SDL_Texture* tiletex;
Entity ents[MAX_ENTITIES] = {0}; // TODO does this zero out the array?
SDL_Texture* txtTex;
u32 g_time;
f32 dt;
f32 accumulator;
bool run;

std::vector<Layer*> layerStack;
ImGuiLayer* igLayer;

void main_loop();

int main(int argc, char* args[])
{
    // SDL SETUP ///////////////////////////////////////////////////////////////
    rw = new RenderWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    tex = IMG_LoadTexture(rw->renderer, "res/character.png");
    SDL_ERROR(tex);

    tiletex = IMG_LoadTexture(rw->renderer, "res/gravetiles.png");
    SDL_ERROR(tiletex);

    //memset(ents, 0, sizeof(ents));
    ents[0] = { .active = true, .freed = false,
                .flags = (u32) EntityFlag::PLAYER_CONTROLLED,
                .position = {0,0,0}, .orient = 0, .renderLayer = 1,
                .sprite{{0,0,16,32}, tex, {0,0}}};
    ents[1] = { .active = true, .freed = false, .flags = 0,
                .position = {0,16,0}, .orient = 0, .renderLayer = 0,
                .sprite{{32,32,352,224}, tiletex, {0,0}}};
    ents[2] = { .active = true, .freed = false, .flags = 0,
                .position = {500,800,0}, .orient = 0, .renderLayer = 1,
                .sprite{{64,64,16,32}, tex, {0,0}}};

    for (u32 i = 3; i < 100; i++)
    {
        for (u32 j = 1; j < 100; j++)
        {
            ents[i*j] = { .active = true, .freed = false,
                          .flags = (u32) EntityFlag::PLAYER_CONTROLLED | (u32) EntityFlag::IS_ANIMATED,
                          .position = {13 * i, 10 * j,0}, .orient = 3, .renderLayer = 1,
                          .sprite{{0,0,16,32}, tex, {0,0}, SDL_FLIP_VERTICAL},
                          .anim{ {{0,0,16,32}, {16,0,16,32}, {32,0,16,32}, {48,0,16,32}},
                                 1.0f, true } };
        }
    }

    // Font Test
    TTF_Init();
    TTF_Font* font        = TTF_OpenFont( "res/gothic.ttf", 40 );
    std::string text      = "Linebreaks are working.\nLook at all these perfect linebreaks.\n"
                            "This is achieved wtih TTF_RenderText_Blended_Wrapped()\n"
                            "Another line here.";

    SDL_Color textColor   = {150,160,100,230};
    //SDL_Surface* textSurf = TTF_RenderText_Solid(font, text.c_str(), textColor);
    SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font, text.c_str(),
                                                           textColor, 800);
    SDL_ERROR(textSurf);
    txtTex   = SDL_CreateTextureFromSurface(rw->renderer, textSurf);
    SDL_ERROR(txtTex);

#ifdef IMGUI
    igLayer = new ImGuiLayer();
    layerStack.push_back(igLayer);
    igLayer->OnAttach();
#endif

    // main loop ///////////////////////////////////////////////////////////////
    g_time = 0;
    dt = 0;
    accumulator = 0;
    run = true;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, -1, 1);
#else
    main_loop();
#endif

    // CLEANUP /////////////////////////////////////////////////////////////////
#ifdef IMGUI
    igLayer->OnDetach();
#endif

    SDL_DestroyRenderer(rw->renderer);
    SDL_DestroyWindow(rw->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void main_loop()
{
#ifndef __EMSCRIPTEN__
    while (run)
#endif
    {
        dt = (SDL_GetTicks() - g_time) / 1000.f;
        g_time = SDL_GetTicks();
        accumulator += (g_time/1000.f);

        // TODO not working
        //while (accumulator > TIME_PER_FRAME) {
        //    accumulator -= TIME_PER_FRAME;
        //}
        // EVENT HANDLING //////////////////////////////////////////////////////
        Event evn;
        while (SDL_PollEvent(&evn.evn))
        {
#ifdef IMGUI
            igLayer->OnEvent(evn);
#endif
            // TODO use layerstack to revere iterate and propagate events:
            /*
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
            (e.Handled) break;
			(*it)->OnEvent(e);
		}
		*/
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
                  Player::handleEvent(evn, /*io,*/ ents[i]); // TODO remove imgui dependency
            }

            switch (evn.evn.type) {
            case SDL_QUIT: run = false;
                break;
            case SDL_WINDOWEVENT:
                if (evn.evn.window.type == SDL_WINDOWEVENT_CLOSE) run = false;
                break;
            }
        }

        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].flags & (u32) EntityFlag::IS_ANIMATED)
                ents[i].sprite.box = Animator::animate(dt, ents[i].anim);
        }

#ifdef IMGUI
        // DEAR IMGUI //////////////////////////////////////////////////////////
        igLayer->Begin();

        ImGui::ShowDemoWindow();
        ImGui::Begin("Hello World");
        ImGui::Text("TICKS: %d", g_time);
        ImGui::Text("ACCU: %f", accumulator);
        ImGui::Text("DT: %f", dt);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
#endif

        // RENDERING ///////////////////////////////////////////////////////////
        SDL_RenderClear(rw->renderer);

        u32 maxlayer = 0;
        for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
        {
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                if (!ents[i].active) continue;
                if (ents[i].renderLayer != l) continue;
                rw->render(ents[i].sprite, ents[i].position, 1.5f, ents[i].sprite.flip);

                if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
            }
            // no need to go through all renderlayers
            if (l > maxlayer) break;
        }

        int w,h;
        SDL_QueryTexture(txtTex, NULL, NULL, &w, &h);
        SDL_Rect dst{100,600,w,h};
        SDL_RenderCopy(rw->renderer, txtTex, NULL, &dst);

#ifdef IMGUI
        for (auto l : layerStack)
            l->OnImGuiRender();
        igLayer->End();
#endif
        SDL_RenderPresent(rw->renderer);
    }

}
