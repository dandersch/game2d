#include "game.h"

#include "memory.h"
// TODO move this somewhere else ///////////////////////////////////////////////////////////////////
struct platform_window_t;

struct game_state_t
{
    b32 initialized = true;            // used by game
    //platform_api_t platform;         // unused
    platform_window_t* window;         // used by game, layer, resourcemgr
    b32 game_running = true;           // used by game

    Entity ents[MAX_ENTITIES] = {};    // used by entity, game, layer
    u32    temp_count         = 0;     // used by entity
    Tile   tiles[MAX_TILES]   = {};    // used by entity, game
    u32    tile_count         = 0;     // used by entity

    game_input_t game_input;           // used by input, layer
    u32 actionState;                   // used by input, rewind, player

    f32 last_frame_time;               // global because we need it in the
    f32 cycles_left_over;              // "main_loop" used for emscripten

    Camera cam = {};                   // used by game, layer
    bool debugDraw;                    // used by layer
    Entity* focusedEntity;             // used by layer
    rect_t focusArrow = {64,32,16,32}; // used by layer TODO hardcoded

    // commandprocessor
    u32 cmdIdx = 0;            // used by rewind, layer

    // reset
    bool isRewinding;          // used by rewind, layer
    f32 loopTime;              // used by rewind, layer (debug)

    Entity* entity_to_place = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

extern game_state_t* state;     // TODO some cpp files depend on this
extern platform_api_t platform; // TODO some cpp files depend on this
// we use a unity build, see en.wikipedia.org/wiki/Unity_build
#include "camera.cpp"
#include "physics.cpp"
#include "entity.cpp"
#include "input.cpp"
#include "player.cpp"
#include "resourcemgr.cpp"
#include "rewind.cpp"
#include "levelgen.cpp"

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

game_state_t* state = nullptr;
platform_api_t platform = {0};
const u32 SCREEN_WIDTH  = 1280;
const u32 SCREEN_HEIGHT =  960;
const char* GAME_LEVEL = "res/map_level01.json";
const int MAX_RENDER_LAYERS = 100;

texture_t* test_tex = nullptr;
//rect_t dst = {0.5f, 0.5f, 0.1f, 0.1f};
rect_t test_dst = {200, 200, 200, 100};

#include "interface.h"
ui_t ui_ctx = {};
sprite_t skelly_sprite; // TODO hardcoded
Entity entity_skeleton = {};

// TODO pass in game_input_t too (?)
extern "C" void game_main_loop(game_state_t* game_state, platform_api_t platform_api)
{
    state    = game_state;
    platform = platform_api;

    // INIT ///////////////////////////////////////////////////////////////////////////////
    if (!state->initialized)
    {
        state = new (game_state) game_state_t();
        //mem_arena(&game_arena,)
        //state->entity_arena
        state = game_state;
        state->window = platform.window_open("hello game", SCREEN_WIDTH, SCREEN_HEIGHT);
        { // layer_game_init:
            if (!levelgen_level_load(GAME_LEVEL, nullptr, MAX_ENTITIES, state)) exit(1);
            physics_init();
        }

        test_tex = resourcemgr_texture_load("button.png", state);

        skelly_sprite.box   = {0,0,16,32};
        skelly_sprite.tex   = resourcemgr_texture_load("tileset.png", state);
        skelly_sprite.pivot = {0.5f, 0.5f};

        entity_skeleton.active       = true;
        entity_skeleton.freed        = false;
        entity_skeleton.sprite.box   = skelly_sprite.box;
        entity_skeleton.sprite.pivot = {0.5f, 0.5f};
        entity_skeleton.state        = ENT_STATE_MOVE;
        entity_skeleton.sprite.tex   = skelly_sprite.tex;
        entity_skeleton.orient       = ENT_ORIENT_DOWN;
        entity_skeleton.collider     = skelly_sprite.box;
        entity_skeleton.flags       |= ENT_FLAG_IS_COLLIDER;
        entity_skeleton.flags       |= ENT_FLAG_CMD_CONTROLLED;
        entity_skeleton.flags       |= ENT_FLAG_IS_REWINDABLE;
        Rewind::initializeFrames(entity_skeleton);
        command_init(entity_skeleton);
    }

    // TIMESTEP ////////////////////////////////////////////////////////////
    f32 curr_time = platform.ticks() / 1000.f;
    f32 update_iterations = ((curr_time - state->last_frame_time) + state->cycles_left_over);
    f32 dt = UPDATE_INTERVAL;

    if (update_iterations > (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL)) {
        update_iterations = (MAX_CYCLES_PER_FRAME * UPDATE_INTERVAL);
    }

    while (update_iterations > UPDATE_INTERVAL)
    {
        update_iterations -= UPDATE_INTERVAL;

        // EVENT HANDLING //////////////////////////////////////////////////////////////////////////
        platform.event_loop(&state->game_input);

        // quit game on escape
        if (input_pressed(state->game_input.keyboard.keys['\e'])) state->game_running = false;

        { // game event handling
            v3i mouse_pos   = state->game_input.mouse.pos;
            if (state->entity_to_place)
                state->entity_to_place->setPivPos({(f32) mouse_pos.x, (f32) mouse_pos.y, 1.0f});

            if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]))
            {
                v3f click       = camera_screen_to_world(state->cam, { (f32) mouse_pos.x, (f32) mouse_pos.y, 0 });
                v2i clickpoint  = {(i32) click.x, (i32) click.y};

                // get 'clicked on' playable entity
                for (u32 i = 0; i < MAX_ENTITIES; i++)
                {
                    auto ents = state->ents;
                    if (!ents[i].active) continue;
                    if (!(ents[i].flags & ENT_FLAG_CMD_CONTROLLED)) continue;
                    rect_t   coll       = ents[i].getColliderInWorld();
                    if (utils_point_in_rect(clickpoint, coll)) // TODO
                    {
                        if (state->focusedEntity)
                        {
                            state->focusedEntity->flags ^= ENT_FLAG_PLAYER_CONTROLLED;
                            state->focusedEntity->flags |= ENT_FLAG_CMD_CONTROLLED;
                        }
                        ents[i].flags |= ENT_FLAG_PLAYER_CONTROLLED;
                        ents[i].flags ^= ENT_FLAG_CMD_CONTROLLED;
                        state->focusedEntity = &ents[i];
                    }
                }

                // try to place an entity if we have one selected
                if (state->entity_to_place)
                {
                    printf("Placed entity!\n");
                    state->entity_to_place->setPivPos({(f32)clickpoint.x, (f32)clickpoint.y, 1.0f});
                    EntityMgr::copyEntity(*state->entity_to_place);
                    state->entity_to_place->setPivPos({(f32) mouse_pos.x, (f32) mouse_pos.y, 1.0f});
                }
                else
                {
                    // TODO interpolate TODO breaks w/ zooming
                    // put camera where clicked
                    state->cam.rect.x = click.x - (state->cam.rect.w/2.f);
                    state->cam.rect.y = click.y - (state->cam.rect.h/2.f);
                    // TODO put cursor where clicked
                    //SDL_WarpMouseInWindow(globals.rw->window, (cam.rect.w/2.f), (cam.rect.h/2.f));
                }
            }

            if (state->game_input.keyboard.key_up.is_down)    state->cam.rect.y -= 5;
            if (state->game_input.keyboard.key_down.is_down)  state->cam.rect.y += 5;
            if (state->game_input.keyboard.key_left.is_down)  state->cam.rect.x -= 5;
            if (state->game_input.keyboard.key_right.is_down) state->cam.rect.x += 5;

            // TODO zoom in/out on mouse scroll
            if (state->game_input.keyboard.f_key_pressed[2]) state->cam.rect.w *= 0.5f;
            if (state->game_input.keyboard.f_key_pressed[3]) state->cam.rect.w *= 2.0f;
        }

        // UPDATE LOOP /////////////////////////////////////////////////////////////////////////////
        { // layer_game_update(dt);
            EntityMgr::freeTemporaryStorage();
            input_update();
            Reset::update(dt); // TODO fixed delta time

            /* test our immediate mode ui */
            // ui_begin():
            ui_ctx.mouse_pos     = {state->game_input.mouse.pos.x,state->game_input.mouse.pos.y};
            ui_ctx.mouse_pressed = input_pressed(state->game_input.mouse.buttons[0]);
            ui_ctx.btn_texture   = test_tex;
            ui_ctx.curr_focus    = __COUNTER__; // zero
            // i32 button_count = 2;
            // for (i32 i = 0; i < button_count; i++)
            // {
            //     rect_t button_dst = {1000, 800 - (i*150), 200, 100};
            //     if (ui_button(&ui_ctx, button_dst, NULL, 10+i))
            //     {
            //         printf("btn %u!\n", i);
            //     }
            // }
            if (ui_button(&ui_ctx, test_dst, &skelly_sprite, __COUNTER__))
            {
                printf("pressed btn 1!\n");
                state->entity_to_place = &entity_skeleton;
            }
            if (ui_button(&ui_ctx, {test_dst.x, test_dst.y + 200, test_dst.w, test_dst.h}, NULL, __COUNTER__))
            {
                printf("pressed btn 2!\n");
                state->entity_to_place = nullptr;
            }
            // ui_end();

            auto tiles = EntityMgr::getTiles();

            // TODO find out if it matters if we do everything in one loop for one
            // entity vs. every "system" has its own loop
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                auto& ent = state->ents[i];

                // PLAYER CONTROLLER ///////////////////////////////////////////////////////////////////////
                if (!state->isRewinding && ent.active)
                {
                    if (ent.flags & ENT_FLAG_PLAYER_CONTROLLED)
                    {
                        player_update(dt, ent);
                    }
                }

                // COMMAND REPLAY //////////////////////////////////////////////////////////////////////////
                if (!state->isRewinding && ent.active)
                {
                    if (ent.flags & ENT_FLAG_CMD_CONTROLLED)
                    {
                        command_replay(ent);
                    }
                }

                // COLLISION CHECKING //////////////////////////////////////////////////////////////////////
                if (!state->isRewinding && ent.active && i != MAX_ENTITIES)
                {
                    bool collided = false;
                    if ((ent.flags & ENT_FLAG_IS_COLLIDER)
                        /*
                        && ((ent.flags & ENT_FLAG_PLAYER_CONTROLLED) ||
                            (ent.flags & ENT_FLAG_CMD_CONTROLLED) ||
                            (ent.flags & ENT_FLAG_ATTACK_BOX) || // TODO improve this
                            (ent.flags & ENT_FLAG_PICKUP_BOX))
                            */
                        )
                    {
                        for (u32 k = 0; k < EntityMgr::getTileCount(); k++)
                        {
                            if (!tiles[k].collidable) continue;
                            collided |= physics_check_collision_with_tile(ent, tiles[k]);
                            if (collided) break;
                        }

                        for (u32 j = i+1; j < MAX_ENTITIES; j++)
                        {
                            Entity& e2 = state->ents[j];
                            if (!e2.active) continue;
                            if ((e2.flags & ENT_FLAG_IS_COLLIDER))
                                collided |= physics_check_collision(ent, e2);
                        }
                    }
                    // TODO should we set movement to zero here if collided?
                    if (!collided)
                    {
                        ent.position = {ent.position.x + ent.movement.x,
                                        ent.position.y + ent.movement.y,
                                        ent.position.z + ent.movement.z};
                    }
                }

                // TIME REWIND /////////////////////////////////////////////////////////////////////////////
                if ((ent.flags & ENT_FLAG_IS_REWINDABLE))
                {
                    Rewind::update(dt, ent);
                }

                // NOTE animation should probably be last after input & collision etc.
                // TODO animation can crash if IS_ANIMATED entities don't have filled arrays..
                if (ent.flags & ENT_FLAG_IS_ANIMATED)
                {
                    //ent.sprite.box = Animator::animate(dt, ent.anim);
                    ent.sprite.box = animation_update(&ent.anim, ent.clips, ent.clip_count, dt);
                }
            }

            // after loop update
            command_on_update_end();
        } // update loop
    }

    state->cycles_left_over = update_iterations;
    state->last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////////////////////////////

    platform.renderer.push_clear({});
    { // layer_game_render();
        Entity* ents = state->ents;
        Tile* tiles = EntityMgr::getTiles();

        // RENDER TILES ////////////////////////////////////////////////////////////////////////////
        // TODO tilemap culling
        for (u32 i = 0; i < EntityMgr::getTileCount(); i++)
        {
            platform.renderer.push_sprite(tiles[i].sprite.tex, tiles[i].sprite.box,
                                          camera_world_to_screen(state->cam, tiles[i].position),
                                          state->cam.scale);

            if (state->debugDraw)
            {
                auto pos = camera_world_to_screen(state->cam, tiles[i].position);
                platform.debug_draw(state->window, tiles[i].collider, pos, {0,0,0,255}, 1.0f);
            }
        }

        // RENDER ENTITIES /////////////////////////////////////////////////////////////////////////
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].sprite.tex != nullptr) // workaround for temporary invisible entities,
                                               // TODO instead we should maybe just have an ENT_FLAG_RENDERED
                                               // that we check here
                platform.renderer.push_sprite(ents[i].sprite.tex, ents[i].sprite.box,
                                              camera_world_to_screen(state->cam, ents[i].position),
                                              state->cam.scale);

            if (state->debugDraw)
            {
                // change color depending on entity flags
                color_t c = {0,0,0,255}; // TODO get this out
                if (ents[i].flags & ENT_FLAG_ATTACK_BOX) c = {255,100,100,255};
                if (ents[i].flags & ENT_FLAG_PICKUP_BOX) c = {100,255,100,255};

                auto ent_pos = camera_world_to_screen(state->cam, ents[i].position);
                platform.debug_draw(state->window, ents[i].collider, ent_pos, c, ents[i].scale);
            }
        }

        // draw focusarrow on focused entity TODO hardcoded & very hacky
        if (state->focusedEntity)
        {
            sprite_t arrow_sprite = { state->focusArrow, ents[0].sprite.tex };
            auto pos = camera_world_to_screen(state->cam, state->focusedEntity->position);
            platform.renderer.push_sprite(arrow_sprite.tex, arrow_sprite.box, pos, state->cam.scale);
        }

        if (state->entity_to_place)
        {
            platform.renderer.push_sprite(state->entity_to_place->sprite.tex, state->entity_to_place->sprite.box,
                                          state->entity_to_place->position, state->entity_to_place->scale);
        }
    }

    ui_render(&ui_ctx);
    platform.renderer.push_present({});
    platform.render(state->window);

    // game_quit:
    if (!state->game_running)
    {
        platform.window_close(state->window);
        platform.quit();
        exit(0);
    }
}
