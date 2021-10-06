#include "layer.h"

// IMGUI include TODO maybe move this somewhere else
#ifdef IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "imgui_impl_sdl.h"
#endif

#include "collision.h"
#include "animation.h"
#include "input.h"
#include "resourcemgr.h"
#include "rewind.h"
#include "camera.h"
#include "player.h"
#include "collision.h"
#include "globals.h"
#include "utils.h"

#include "platform.h"

#include "memory.h"
extern game_state_t* state;

// GAMELAYER ///////////////////////////////////////////////////////////////////////////////////////
static const int MAX_RENDER_LAYERS = 100;

void layer_game_init()
{
    if (!platform.level_load("res/tiletest.tmx", nullptr, MAX_ENTITIES, state,
                             &resourcemgr_texture_load, &resourcemgr_font_load,
                             &EntityMgr::copyEntity, &EntityMgr::createTile,
                             &Rewind::initializeFrames, &CommandProcessor::initialize))
        exit(1);
}

void layer_game_handle_event()
{
    if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
    {
        v3i  mouse_pos = state->game_input.mouse.pos;
        auto click     = camera_screen_to_world(state->cam, {(f32) mouse_pos.x,
                                                             (f32) mouse_pos.y, 0});

        printf("calculated world pos at: ");
        printf("%f ",  click.x);
        printf("%f\n", click.y);

        // TODO interpolate
        // TODO breaks w/ zooming
        // put camera where clicked
        state->cam.rect.x = click.x - (state->cam.rect.w/2.f);
        state->cam.rect.y = click.y - (state->cam.rect.h/2.f);
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
                if (state->focusedEntity)
                {
                    state->focusedEntity->flags ^= (u32) EntityFlag::PLAYER_CONTROLLED;
                    state->focusedEntity->flags |= (u32) EntityFlag::CMD_CONTROLLED;
                }
                ents[i].flags |= (u32) EntityFlag::PLAYER_CONTROLLED;
                ents[i].flags ^= (u32) EntityFlag::CMD_CONTROLLED;
                state->focusedEntity = &ents[i];
            }
        }
    }

    if (state->game_input.keyboard.key_up.is_down)    state->cam.rect.y -= 5;
    if (state->game_input.keyboard.key_down.is_down)  state->cam.rect.y += 5;
    if (state->game_input.keyboard.key_left.is_down)  state->cam.rect.x -= 5;
    if (state->game_input.keyboard.key_right.is_down) state->cam.rect.x += 5;

    // TODO zoom in/out on mouse scroll
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

        // PLAYER CONTROLLER ///////////////////////////////////////////////////////////////////////
        if (!state->isRewinding && ent.active)
        {
            if (ent.flags & (u32) EntityFlag::PLAYER_CONTROLLED)
            {
                player_update(dt, ent);
            }
        }

        // COMMAND REPLAY //////////////////////////////////////////////////////////////////////////
        if (!state->isRewinding && ent.active)
        {
            if (ent.flags & (u32) EntityFlag::CMD_CONTROLLED)
            {
                CommandProcessor::replay(ent);
            }
        }

        // COLLISION CHECKING //////////////////////////////////////////////////////////////////////
        if (!state->isRewinding && ent.active)
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

        // TIME REWIND /////////////////////////////////////////////////////////////////////////////
        if ((ent.flags & (u32) EntityFlag::IS_REWINDABLE))
        {
            Rewind::update(dt, ent);
        }

        // NOTE animation should probably be last after input & collision etc.
        // TODO animation can crash if IS_ANIMATED entities don't have filled arrays..
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
    Entity* ents = EntityMgr::getArray();
    Tile* tiles = EntityMgr::getTiles();
    for (u32 l = 0; l < MAX_RENDER_LAYERS; l++) // TODO use z coordinate and let
                                                // renderer sort w/ a cmd key
    {
        // RENDER TILES ////////////////////////////////////////////////////////////////////////////
        // TODO tilemap culling
        for (u32 i = 0; i < EntityMgr::getTileCount(); i++)
        {
            if (tiles[i].renderLayer != l) continue;
            platform.render_sprite(state->window, tiles[i].sprite,
                                   camera_world_to_screen(state->cam, tiles[i].position),
                                   state->cam.scale, tiles[i].sprite.flip);
            if (tiles[i].renderLayer > maxlayer) maxlayer = tiles[i].renderLayer;

            if (state->debugDraw)
            {
                auto pos = camera_world_to_screen(state->cam, tiles[i].position);
                rect_t dst = {(int) pos.x + tiles[i].collider.x,
                              (int) pos.y + tiles[i].collider.y,
                              (i32) (tiles[i].collider.w),
                              (i32) (tiles[i].collider.h)};

                // TODO don't draw 'empty' colliders (otherwise it will draw points & lines)
                if (!rect_empty(dst))
                   platform.debug_draw_rect(state->window, &dst);
            }
        }

        // RENDER ENTITIES /////////////////////////////////////////////////////////////////////////
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].renderLayer != l) continue;
            platform.render_sprite(state->window, ents[i].sprite,
                                   camera_world_to_screen(state->cam, ents[i].position),
                                   state->cam.scale, ents[i].sprite.flip);

            if (state->debugDraw)
            {
                // change color depending on entity flags
                color_t c = {0,0,0,255}; // TODO get this out
                if (ents[i].flags & (u32) EntityFlag::ATTACK_BOX) c = {255,100,100,255};
                if (ents[i].flags & (u32) EntityFlag::PICKUP_BOX) c = {100,255,100,255};

                platform.render_set_draw_color(state->window, c.r, c.g, c.b, c.a);
                platform.debug_draw(state->window, ents[i],
                                    camera_world_to_screen(state->cam, ents[i].position));
                platform.render_set_draw_color(state->window, 0, 0, 0, 255);
            }

            if (ents[i].renderLayer > maxlayer) maxlayer = ents[i].renderLayer;
        }
        // no need to go through all renderlayers
        if (l > maxlayer) break;
    }

    // draw focusarrow on focused entity TODO hardcoded & very hacky
    if (state->focusedEntity)
    {
        sprite_t arrow_sprite = { state->focusArrow, ents[0].sprite.tex };
        auto pos = camera_world_to_screen(state->cam, state->focusedEntity->position);
        platform.render_sprite(state->window, arrow_sprite, pos, state->cam.scale, 0);
    }

}

void layer_game_imgui_render()
{
#ifdef IMGUI
    auto& ent = EntityMgr::getArray()[0];

    //ImGui::ShowDemoWindow();
    ImGui::Begin("Hello World");
    ImGui::Text("TICKS: %d", platform.ticks());
    ImGui::Text("DT: %f", state->dt);
    ImGui::Text("CMD IDX: %u", state->cmdIdx);
    ImGui::Text("LOOP TIME: %f", state->loopTime);
    ImGui::Text("IS REWINDING: %u", state->isRewinding);
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

    ImGui::Checkbox("ENABLE DEBUG DRAW", &state->debugDraw);
    ImGui::End();
#endif
}

// MENULAYER ///////////////////////////////////////////////////////////////////////////////////////
//static std::vector<Button> btns;
//static texture_t* btn_inactive_tex;
//static texture_t* btn_hover_tex;
//static texture_t* btn_pressed_tex;
//static texture_t* greyout_tex;
//b32 g_layer_menu_is_active = true;

// create & get needed texs + create buttons & texs for labels
void layer_menu_init()
{
    state->g_layer_menu_is_active = false;

    state->btn_inactive_tex = resourcemgr_texture_load("res/button.png", state);
    state->btn_hover_tex    = resourcemgr_texture_load("res/button_hover.png", state);
    state->btn_pressed_tex  = resourcemgr_texture_load("res/button_pressed.png", state);

    Button b1 = { .label = "CONTINUE", .state = Button::NONE, .box = {800, 475,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };
    Button b2 = { .label = "OPTIONS",  .state = Button::NONE, .box = {800, 600,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };
    Button b3 = { .label = "EXIT",     .state = Button::NONE, .box = {800, 725,300,100},
                  .tex = { state->btn_inactive_tex, state->btn_hover_tex, state->btn_pressed_tex } };

    // ADD CALLBACKS
    b1.callback = [&]() { state->g_layer_menu_is_active = false; };
    b2.callback = []()
    {
        /*printf("Trying to set VSYNC. Set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC))*/;
        /*SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);*/
        /*printf("Now set to: %s \n", SDL_GetHint(SDL_HINT_RENDER_VSYNC));*/
        printf("Options button pressed!\n");
    };
    b3.callback = [&]()  { state->game_running = false; }; // TODO signal to platform layer

    state->btns.push_back(b1);
    state->btns.push_back(b2);
    state->btns.push_back(b3);

    // for greying out game in the background when paused
    state->greyout_tex = resourcemgr_texture_load("res/greyout.png", state);

    // ADD TEXT TO BUTTONS
    platform.font_init();
    font_t* btn_font   = resourcemgr_font_load("res/ubuntumono.ttf", state);
    color_t text_color = {200,200,200,230};
    for (auto& b : state->btns)
    {
        surface_t* text_surf = platform.text_render(btn_font, b.label, text_color, 400);
        b.text_texture = platform.texture_create_from_surface(state->window, text_surf);
        platform.texture_query(b.text_texture, NULL, NULL, &b.text_box.w, &b.text_box.h);
        platform.surface_destroy(text_surf);
    }
    // ResourceManager<TTF_Font*>::free("res/ubuntumono.ttf");
}

void layer_menu_handle_event()
{
    point_t mouse = {state->game_input.mouse.pos.x, state->game_input.mouse.pos.y};

    for (auto& b : state->btns)
    {
        if (point_in_rect(mouse, b.box)) b.state = Button::HOVER;
        else b.state = Button::NONE;
    }

    if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
    {
        for (auto& b : state->btns)
        {
            if (b.state == Button::HOVER)
            {
                //evn->handled = true;
                b.state = Button::PRESSED;
                b.callback();
            }
        }
    }
}

void layer_menu_render()
{
    // grey out background
    platform.render_texture(state->window, state->greyout_tex, NULL, NULL);

    // NOTE framerate-dependant
    // make buttons alpha 'pulsate'
    local i8 incr = 1;
    local u8 alpha = 200;
    alpha += incr;
    if (alpha > 254) incr = -1;
    if (alpha < 150)  incr = 1;

    // TODO dont call sdl render functions ourselves
    for (auto& b : state->btns)
    {
        if (b.state == Button::HOVER)
        {
            platform.texture_set_blend_mode(b.tex[b.state], 1 /*SDL_BLENDMODE_BLEND*/);
            platform.texture_set_alpha_mod(b.tex[b.state], alpha);
            //SDL_SetTextureColorMod(b.tex[b.state],100,100,4);
        }

        platform.render_texture(state->window, b.tex[b.state], NULL, &b.box);
        rect_t text_dst = { b.box.x + b.box.w/2 - b.text_box.w/2,
                           b.box.y + b.box.h/2 - b.text_box.h/2,
                           b.text_box.w, b.text_box.h};
        platform.render_texture(state->window, b.text_texture, NULL, &text_dst);

    }
}

// IMGUILAYER //////////////////////////////////////////////////////////////////////////////////////
void layer_imgui_init() { platform.imgui_init(state->window, SCREEN_WIDTH, SCREEN_HEIGHT); }
void layer_imgui_destroy() { platform.imgui_destroy(); }
void layer_imgui_handle_event(game_input_t* input) { platform.imgui_event_handle(input); }
void layer_imgui_begin() { platform.imgui_begin(state->window); }
void layer_imgui_end() { platform.imgui_end(); }
