#include "gamelayer.h"
#include "command.h"
#include "input.h"
#include "levelgen.h"
#include "renderwindow.h"
#include "reset.h"
#include "rewind.h"

//Entity GameLayer::ents[MAX_ENTITIES] = {0}; // TODO does this zero out the array?

void GameLayer::OnAttach()
{
    // TEST TILE GENERATION ////////////////////////////////////////////////
    // TODO LevelGenerator that can fill the entityarray with static tiles &
    // (items &) characters (maybe without sprites), afterwards fill characters
    // (i.e. entities with flag IS_CHARACTER or sth.) and fill e.g animations of
    // entities with CharacterType SKELETON with "skeleton.tmx"
    LevelGenerator levelgen;
    if (!levelgen.loadLevel("res/tiletest.tmx", nullptr, MAX_ENTITIES))
        exit(1);

    // Font Test
    TTF_Init();
    TTF_Font* font        = TTF_OpenFont( "res/gothic.ttf", 40 );
    std::string text      = "Linebreaks are working.\nLook at all these perfect linebreaks.\n"
                            "This is achieved wtih TTF_RenderText_Blended_Wrapped()\n"
                            "Another line here.";

    SDL_Color textColor   = {150,160,100,230};
    SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font, text.c_str(),
                                                           textColor, 800);
    SDL_ERROR(textSurf);
    txtTex   = SDL_CreateTextureFromSurface(rw->renderer, textSurf);
    SDL_ERROR(txtTex);
}

void GameLayer::OnDetach()
{
}

void GameLayer::OnEvent(Event& event)
{
    SDL_Event evn = event.evn;

    switch (evn.type) {
    case SDL_KEYDOWN:
        switch (evn.key.keysym.sym)
        {
        case SDLK_UP:
            cam.cameraRect.y -= 5;
            break;
        case SDLK_DOWN:
            cam.cameraRect.y += 5;
            break;
        case SDLK_LEFT:
            cam.cameraRect.x -= 5;
            break;
        case SDLK_RIGHT:
            cam.cameraRect.x += 5;
            break;
        }
        break;
    case SDL_MOUSEMOTION:
        break;
    case SDL_MOUSEBUTTONDOWN:
        auto click = cam.screenToWorld({evn.button.x, evn.button.y, 0});
        cam.cameraRect.x = click.x - (cam.cameraRect.w/2);
        cam.cameraRect.y = click.y - (cam.cameraRect.h/2);
        break;
    }

    for (u32 i = 0; i < MAX_ENTITIES; i++)
    {
        auto ents = EntityMgr::getArray();
        if (!ents[i].active) continue;
        if (ents[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED)
            Player::handleEvent(event, ents[i], cam);
    }
}

void GameLayer::OnUpdate(f32 dt)
{
    EntityMgr::freeTemporaryStorage();

    // update input
    Input::update();
    Reset::update(dt); // TODO fixed delta time

    // TODO find out if it matters if we do everything in one loop for one
    // entity vs. every "system" has its own loop
    for (u32 i = 0; i < MAX_ENTITIES; i++)
    {
        auto& ent = EntityMgr::getArray()[i];

        // PLAYER CONTROLLER ///////////////////////////////////////////////////
        if (!Reset::isRewinding && ent.active)
        {
            if (ent.flags & (u32) EntityFlag::PLAYER_CONTROLLED)
            {
              Player::update(dt, ent);
            }
        }

        // COMMAND REPLAY //////////////////////////////////////////////////////
        if (!Reset::isRewinding && ent.active)
        {
            if (ent.flags & (u32) EntityFlag::CMD_CONTROLLED)
            {
                CommandProcessor::replay(ent);
            }
        }

        // COLLISION CHECKING //////////////////////////////////////////////////
        if (!Reset::isRewinding && ent.active)
        {
            bool collided = false;
            if ((ent.flags & (u32) EntityFlag::IS_COLLIDER) &&
                (ent.flags & (u32) EntityFlag::PLAYER_CONTROLLED))
            {
                for (u32 j = i; j < MAX_ENTITIES; j++)
                {
                    Entity& e2 = EntityMgr::getArray()[j];
                    if (!e2.active) continue;
                    if ((e2.flags & (u32) EntityFlag::IS_COLLIDER) && (&ent != &e2))
                        collided = Collision::checkCollision(ent, e2);
                }
            }
            // TODO should we set movement to zero here if collided?
            if (!collided)
            {
                ent.position += ent.movement;
                // TODO interpolate here
                // start = ent.position;
                // end   = ent.position + ent.movement;
                // ent.position = glm::mix(start, end, interpolant)
            }
        }

        // TIME REWIND /////////////////////////////////////////////////////////
        if ((ent.flags & (u32) EntityFlag::IS_REWINDABLE))
        {
            Rewind::update(dt, ent);
        }

        // NOTE animation should probably be last after input & collision etc.
        // TODO animation can crash if IS_ANIMATED entities don't have filled
        // arrays..
        if (ent.flags & (u32) EntityFlag::IS_ANIMATED)
        {
            ent.sprite.box = Animator::animate(dt, ent.anim);
        }
    }

    // after loop update
    CommandProcessor::onEndUpdate();
}

void GameLayer::OnRender()
{
    u32 maxlayer = 0;
    for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
    {
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            auto ents = EntityMgr::getArray();
            if (!ents[i].active) continue;
            if (ents[i].renderLayer != l) continue;
            rw->render(ents[i].sprite, cam.worldToScreen(ents[i].position),
                       1.0f, ents[i].sprite.flip);

            if (debugDraw)
            {
                rw->debugDraw(ents[i], cam.worldToScreen(ents[i].position));
            }

            if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
        }
        // no need to go through all renderlayers
        if (l > maxlayer) break;
    }

    // testing text
    int w,h;
    SDL_QueryTexture(txtTex, NULL, NULL, &w, &h);
    SDL_Rect dst{100,600,w,h};
    SDL_RenderCopy(rw->renderer, txtTex, NULL, &dst);
}

void GameLayer::OnImGuiRender()
{
#ifdef IMGUI
    auto& ent = EntityMgr::getArray()[0];

    ImGui::ShowDemoWindow();
    ImGui::Begin("Hello World");
    ImGui::Text("TICKS: %d", g_time);
    ImGui::Text("ACCU: %f", accumulator);
    ImGui::Text("DT: %f", dt);
    ImGui::Text("CMD IDX: %u", CommandProcessor::cmdIdx);
    ImGui::Text("LOOP TIME: %f", Reset::loopTime);
    ImGui::Text("IS REWINDING: %u", Reset::isRewinding);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    if (ImGui::Button("TOGGLE PLAYER/CMD CONTROL"))
    {
        printf("TOGGLED\n");
        // TODO better way to toggle these bits...
        if (ent.flags & (u32) EntityFlag::PLAYER_CONTROLLED)
        {
            ent.flags ^= (u32) EntityFlag::PLAYER_CONTROLLED;
            ent.flags |= (u32) EntityFlag::CMD_CONTROLLED;
        }
        else if (ent.flags & (u32) EntityFlag::CMD_CONTROLLED)
        {
            ent.flags |= (u32) EntityFlag::PLAYER_CONTROLLED;
            ent.flags ^= (u32) EntityFlag::CMD_CONTROLLED;
        }
    }

    ImGui::Checkbox("ENABLE DEBUG DRAW", &debugDraw);
    ImGui::End();
#endif
}
