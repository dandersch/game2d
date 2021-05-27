#include "gamelayer.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "collision.h"
#include "command.h"
#include "input.h"
#include "levelgen.h"
#include "renderwindow.h"
#include "reset.h"
#include "rewind.h"
#include "item.h"
#include "sound.h"

void GameLayer::OnAttach()
{
    LevelGenerator levelgen;
    if (!levelgen.loadLevel("res/tiletest.tmx", nullptr, MAX_ENTITIES))
        exit(1);
}

void GameLayer::OnDetach()
{
}

void GameLayer::OnEvent(Event& event)
{
    SDL_Event evn = event.sdl;

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
        //cam.cameraRect.x = click.x - (cam.cameraRect.w/2.f);
        //cam.cameraRect.y = click.y - (cam.cameraRect.h/2.f);

        // get 'clicked on' playable entity
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            auto ents = EntityMgr::getArray();
            if (!ents[i].active) continue;
            if (!(ents[i].flags & (u32) EntityFlag::CMD_CONTROLLED)) continue;
            SDL_Point clickpoint = {(i32) click.x, (i32) click.y};
            SDL_Rect coll = ents[i].getColliderInWorld();
            if (SDL_PointInRect(&clickpoint, &coll))
            {
                if (focusedEntity)
                {
                    focusedEntity->flags ^= (u32) EntityFlag::PLAYER_CONTROLLED;
                    focusedEntity->flags |= (u32) EntityFlag::CMD_CONTROLLED;
                }
                ents[i].flags |= (u32) EntityFlag::PLAYER_CONTROLLED;
                ents[i].flags ^= (u32) EntityFlag::CMD_CONTROLLED;
                focusedEntity = &ents[i];
            }
        }
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

    auto tiles = EntityMgr::getTiles();

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

        // ITEM SYSTEM /////////////////////////////////////////////////////////
        if (!Reset::isRewinding && ent.active)
        {
            if (ent.flags & (u32) EntityFlag::IS_ITEM)
            {
                Item::update(dt, ent);
            }
        }

        // COLLISION CHECKING //////////////////////////////////////////////////
        if (!Reset::isRewinding && ent.active)
        {
            bool collided = false;
            if ((ent.flags & (u32) EntityFlag::IS_COLLIDER) &&
                ((ent.flags & (u32) EntityFlag::PLAYER_CONTROLLED) ||
                 (ent.flags & (u32) EntityFlag::CMD_CONTROLLED) ||
                 (ent.flags & (u32) EntityFlag::PICKUP_BOX)))
            {
                for (u32 k = 0; k < EntityMgr::getTileCount(); k++)
                {
                    if (!tiles[k].collidable) continue;
                    collided |= Collision::checkCollisionWithTiles(ent, tiles[k]);
                    if (collided) break;
                }

                for (u32 j = 0; j < MAX_ENTITIES; j++)
                {
                    Entity& e2 = EntityMgr::getArray()[j];
                    if (!e2.active) continue;
                    if ((e2.flags & (u32) EntityFlag::IS_COLLIDER) && (&ent != &e2))
                        collided |= Collision::checkCollision(ent, e2);
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
    auto ents = EntityMgr::getArray();
    auto tiles = EntityMgr::getTiles();
    for (u32 l = 0; l < MAX_RENDER_LAYERS; l++)
    {
        // RENDER TILES ////////////////////////////////////////////////////////
        // TODO tilemap culling
        for (u32 i = 0; i < EntityMgr::getTileCount(); i++)
        {
            if (tiles[i].renderLayer != l) continue;
            rw->render(tiles[i].sprite, cam.worldToScreen(tiles[i].position),
                       1.0f, tiles[i].sprite.flip);
            if (tiles[i].renderLayer > maxlayer) maxlayer = tiles[i].renderLayer;

            if (debugDraw)
            {
                auto pos = cam.worldToScreen(tiles[i].position);
                SDL_Rect dst = {(int) pos.x + tiles[i].collider.x,
                                (int) pos.y + tiles[i].collider.y,
                                (i32) (tiles[i].collider.w),
                                (i32) (tiles[i].collider.h)};

                // don't draw 'empty' colliders (otherwise it will draw points & lines)
                if (!SDL_RectEmpty(&dst)) SDL_RenderDrawRect(rw->renderer, &dst);
            }
        }

        // RENDER ENTITIES /////////////////////////////////////////////////////
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].renderLayer != l) continue;
            rw->render(ents[i].sprite, cam.worldToScreen(ents[i].position),
                       1.0f, ents[i].sprite.flip);

            if (debugDraw)
            {
                // change color depending on entity flags
                SDL_Color c = {0,0,0,255};
                if (ents[i].flags & (u32) EntityFlag::ATTACK_BOX) c = {255,100,100,255};
                if (ents[i].flags & (u32) EntityFlag::PICKUP_BOX) c = {100,255,100,255};

                SDL_SetRenderDrawColor(rw->renderer, c.r, c.g, c.b, c.a);
                rw->debugDraw(ents[i], cam.worldToScreen(ents[i].position));
                SDL_SetRenderDrawColor(rw->renderer, 0,0,0,255);
            }

            if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
        }
        // no need to go through all renderlayers
        if (l > maxlayer) break;
    }

    // draw focusarrow on focused entity TODO hardcoded & very hacky
    if (focusedEntity)
    {
        auto pos     = cam.worldToScreen(focusedEntity->position);
        SDL_Rect dst = {(i32) pos.x, (i32) pos.y, focusedEntity->sprite.box.w, focusedEntity->sprite.box.h};
        SDL_RenderCopy(rw->renderer, ents[0].sprite.tex, &focusArrow, &dst);
    }

}

void GameLayer::OnImGuiRender()
{
#ifdef IMGUI
    auto& ent = EntityMgr::getArray()[0];

    ImGui::ShowDemoWindow();
    ImGui::Begin("Hello World");
    ImGui::Text("TICKS: %d", SDL_GetTicks());
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

    if (ImGui::Button("TOGGLE MUSIC"))
    {
        Sound::toggleMusic();
    }

    ImGui::Checkbox("ENABLE DEBUG DRAW", &debugDraw);
    ImGui::End();
#endif
}
