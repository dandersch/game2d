#include "gamelayer.h"
#include "collision.h"
#include "animation.h"
#include "command.h"
#include "input.h"
#include "levelgen.h"
#include "platform.h"
#include "reset.h"
#include "rewind.h"
#include "camera.h"
#include "entitymgr.h"
#include "player.h"
#include "collision.h"
#include "globals.h"
#include "utils.h"

static const int MAX_RENDER_LAYERS = 100;

static Camera cam;
static bool debugDraw = false;
static Entity* focusedEntity = nullptr;
static rect_t focusArrow = {64,32,16,32}; // TODO hardcoded

void layer_game_init()
{
    if (!levelgen_load_level("res/tiletest.tmx", nullptr, MAX_ENTITIES))
        exit(1);
}

// TODO platform code
void layer_game_handle_event()
{
    if (input_pressed(globals.game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
    {
        v3i  mouse_pos = globals.game_input.mouse.pos;
        auto click     = camera_screen_to_world(cam, {(f32) mouse_pos.x,
                                                      (f32) mouse_pos.y, 0});

        printf("calculated world pos at: ");
        printf("%f ",  click.x);
        printf("%f\n", click.y);

        // TODO interpolate
        // TODO breaks w/ zooming
        // put camera where clicked
        cam.rect.x = click.x - (cam.rect.w/2.f);
        cam.rect.y = click.y - (cam.rect.h/2.f);
        // TODO put cursor where clicked
        //SDL_WarpMouseInWindow(globals.rw->window, (cam.rect.w/2.f), (cam.rect.h/2.f));

        // get 'clicked on' playable entity
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            auto ents = EntityMgr::getArray();
            if (!ents[i].active) continue;
            if (!(ents[i].flags & (u32) EntityFlag::CMD_CONTROLLED)) continue;
            point_t  clickpoint = {(i32) click.x, (i32) click.y};
            rect_t   coll       = ents[i].getColliderInWorld();
            if (point_in_rect(clickpoint, coll)) // TODO
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
    }
        // case EVENT_KEYDOWN:
        // {
        //     switch (evn->key.keycode)
        //     {
        //         case KEY_UP:    { cam.rect.y -= 5; } break;
        //         case KEY_DOWN:  { cam.rect.y += 5; } break;
        //         case KEY_LEFT:  { cam.rect.x -= 5; } break;
        //         case KEY_RIGHT: { cam.rect.x += 5; } break;
        //     }
        // }    break;
        // case EVENT_MOUSEWHEEL: {
        //     //if (evn.wheel.y == -1) cam.rect.w *= 0.5f;
        //     //if (evn.wheel.y ==  1) cam.rect.w *= 2.0f;
        // } break;
}

void layer_game_update(f32 dt)
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
                player_update(dt, ent);
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
                ent.position = {ent.position.x + ent.movement.x,
                                ent.position.y + ent.movement.y,
                                ent.position.z + ent.movement.z};
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
            //ent.sprite.box = Animator::animate(dt, ent.anim);
            ent.sprite.box = animation_update(&ent.anim, ent.clips, ent.clip_count, dt);
        }
    }

    // after loop update
    CommandProcessor::onEndUpdate();
}

void layer_game_render()
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
            platform_render_sprite(globals.window, tiles[i].sprite,
                                   camera_world_to_screen(cam, tiles[i].position),
                                   cam.scale, tiles[i].sprite.flip);
            if (tiles[i].renderLayer > maxlayer) maxlayer = tiles[i].renderLayer;

            if (debugDraw)
            {
                auto pos = camera_world_to_screen(cam, tiles[i].position);
                rect_t dst = {(int) pos.x + tiles[i].collider.x,
                              (int) pos.y + tiles[i].collider.y,
                              (i32) (tiles[i].collider.w),
                              (i32) (tiles[i].collider.h)};

                // TODO don't draw 'empty' colliders (otherwise it will draw points & lines)
                if (!rect_empty(dst))
                   platform_debug_draw_rect(globals.window, &dst);
            }
        }

        // RENDER ENTITIES /////////////////////////////////////////////////////
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].renderLayer != l) continue;
            platform_render_sprite(globals.window, ents[i].sprite,
                                   camera_world_to_screen(cam, ents[i].position),
                                   cam.scale, ents[i].sprite.flip);

            if (debugDraw)
            {
                // change color depending on entity flags
                color_t c = {0,0,0,255}; // TODO get this out
                if (ents[i].flags & (u32) EntityFlag::ATTACK_BOX) c = {255,100,100,255};
                if (ents[i].flags & (u32) EntityFlag::PICKUP_BOX) c = {100,255,100,255};

                platform_render_set_draw_color(globals.window, c.r, c.g, c.b, c.a);
                platform_debug_draw(globals.window, ents[i],
                                    camera_world_to_screen(cam, ents[i].position));
                platform_render_set_draw_color(globals.window, 0, 0, 0, 255);
            }

            if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
        }
        // no need to go through all renderlayers
        if (l > maxlayer) break;
    }

    // draw focusarrow on focused entity TODO hardcoded & very hacky
    if (focusedEntity)
    {
        sprite_t arrow_sprite = { focusArrow, ents[0].sprite.tex };
        auto pos = camera_world_to_screen(cam, v3f{focusedEntity->position});
        platform_render_sprite(globals.window, arrow_sprite,
                               camera_world_to_screen(cam, focusedEntity->position), cam.scale);
    }

}

void layer_game_imgui_render()
{
#ifdef IMGUI
    auto& ent = EntityMgr::getArray()[0];

    ImGui::ShowDemoWindow();
    ImGui::Begin("Hello World");
    ImGui::Text("TICKS: %d", platform_ticks());
    ImGui::Text("DT: %f", globals.dt);
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
