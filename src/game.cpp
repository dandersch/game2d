#include "game.h"

#include "memory.h"
// TODO move this somewhere else ///////////////////////////////////////////////////////////////////
struct platform_window_t;
struct game_state_t
{
    b32 initialized = true;            // used by game
    platform_window_t* window;         // used by game, layer, resourcemgr
    b32 game_running = true;           // used by game

    game_input_t game_input;           // used by input, layer
    u32 actionState;                   // used by input, rewind, player

    Entity ents[MAX_ENTITIES] = {};    // used by entity, game, layer
    u32    temp_count         = 0;     // used by entity
    Tile   tiles[MAX_TILES]   = {};    // used by entity, game
    u32    tile_count         = 0;     // used by entity

    f32 last_frame_time;               // global because we need it in the
    f32 cycles_left_over;              // "main_loop" used for emscripten

    Camera cam = {};                   // used by game, layer

    // replay ctx
    b32 isRewinding;           // used by rewind, layer
    f32 loopTime;              // used by rewind, layer (debug)
    u32 cmdIdx = 0;            // used by rewind, layer

    Entity* focusedEntity;             // used by layer
    rect_t focusArrow = {64,32,16,32}; // used by layer TODO hardcoded
    Entity* entity_to_place = nullptr;
};
enum action_e
{
    ACTION_MOVE_UP    = (1 << 0),
    ACTION_MOVE_DOWN  = (1 << 1),
    ACTION_MOVE_RIGHT = (1 << 2),
    ACTION_MOVE_LEFT  = (1 << 3),
    ACTION_PICKUP     = (1 << 4),
    ACTION_ATTACK     = (1 << 5),
    ACTION_RESTART    = (1 << 6),
};
////////////////////////////////////////////////////////////////////////////////////////////////////

// we use a unity build, see en.wikipedia.org/wiki/Unity_build
#include "camera.cpp"
#include "physics.cpp"
#include "entity.cpp"
#include "player.cpp"
#include "rewind.cpp"
#include "resourcemgr.cpp"
#include "parser.cpp"
#include "levelgen.cpp"

// TIMESTEP constants
#define MAXIMUM_FRAME_RATE 60
#define MINIMUM_FRAME_RATE 60
#define UPDATE_INTERVAL (1.0 / MAXIMUM_FRAME_RATE)
#define MAX_CYCLES_PER_FRAME (MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE)

const u32 SCREEN_WIDTH  = 1280;
const u32 SCREEN_HEIGHT =  960;
const char* GAME_LEVEL = "res/map_level01.json";
const int MAX_RENDER_LAYERS = 100;

#include "interface.h"
ui_t ui_ctx = {};
Entity ui_entities[9] = {};

// TODO pass in game_input_t too (?)
extern "C" void game_main_loop(game_state_t* state, platform_api_t platform)
{
    // INIT ///////////////////////////////////////////////////////////////////////////////
    if (!state->initialized)
    {
        state = new (state) game_state_t();
        state->window = platform.window_open("hello game", SCREEN_WIDTH, SCREEN_HEIGHT);
        { // layer_game_init:
            if (!levelgen_level_load(GAME_LEVEL, MAX_ENTITIES, &platform, state)) exit(1);
            physics_init();
        }

        ui_init(&ui_ctx, &platform, state->window);
        ui_entities[0] = create_entity_from_file("skeleton.ent", &platform, state->window);
        ui_entities[1] = create_entity_from_file("necromancer.ent", &platform, state->window);
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
            v3f click       = camera_screen_to_world(state->cam, { (f32) mouse_pos.x, (f32) mouse_pos.y, 0 });
            //printf("mouse: %f, %f \n", (f32) mouse_pos.x, (f32) mouse_pos.y);
            //printf("click: %f, %f \n", click.x, click.y);
            if (state->entity_to_place)
            {
                f32 clamped_x_pos = (roundf(mouse_pos.x / 16.0f) * 16.0f);
                f32 clamped_y_pos = (roundf(mouse_pos.y / 16.0f) * 16.0f);
                state->entity_to_place->setPivPos({clamped_x_pos, clamped_y_pos, 1.0f});
                //state->entity_to_place->position   = {clamped_x_pos - 8, clamped_y_pos - 24, 1.0f};
                //state->entity_to_place->position = {(f32) mouse_pos.x - 8, (f32) mouse_pos.y - 24, 1.0f};
            }

            if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT]) && !ui_ctx.curr_focus)
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
                    f32 clamped_x_pos = (roundf(mouse_pos.x / 16.0f) * 16.0f);
                    f32 clamped_y_pos = (roundf(mouse_pos.y / 16.0f) * 16.0f);
                    state->entity_to_place->setPivPos(camera_screen_to_world(state->cam, {clamped_x_pos, clamped_y_pos, 0 }));
                    EntityMgr::copyEntity(*state->entity_to_place, state);
                    //state->entity_to_place->setPivPos({(f32) mouse_pos.x, (f32) mouse_pos.y, 1.0f});
                }
                else
                {
                    // TODO interpolate TODO breaks w/ zooming
                    // put camera where clicked
                    state->cam.rect.left = click.x - (state->cam.rect.w/2.f);
                    state->cam.rect.top  = click.y - (state->cam.rect.h/2.f);
                    // TODO put cursor where clicked
                    //SDL_WarpMouseInWindow(globals.rw->window, (cam.rect.w/2.f), (cam.rect.h/2.f));
                }
            }

            if (state->game_input.keyboard.key_up.is_down)    state->cam.rect.top  -= 5;
            if (state->game_input.keyboard.key_down.is_down)  state->cam.rect.top  += 5;
            if (state->game_input.keyboard.key_left.is_down)  state->cam.rect.left -= 5;
            if (state->game_input.keyboard.key_right.is_down) state->cam.rect.left += 5;

            // zoom in/out on mouse scroll
            if (state->game_input.mouse.wheel < 0) state->cam.rect.w *= 0.5f;
            if (state->game_input.mouse.wheel > 0) state->cam.rect.w *= 2.0f;
        }

        // UPDATE LOOP /////////////////////////////////////////////////////////////////////////////
        { // layer_game_update(dt);
            EntityMgr::freeTemporaryStorage(state);
            { // input_update();
                state->actionState = 0;

                if (input_down(state->game_input.keyboard.keys['w'])) state->actionState |= ACTION_MOVE_UP;
                if (input_down(state->game_input.keyboard.keys['a'])) state->actionState |= ACTION_MOVE_LEFT;
                if (input_down(state->game_input.keyboard.keys['s'])) state->actionState |= ACTION_MOVE_DOWN;
                if (input_down(state->game_input.keyboard.keys['d'])) state->actionState |= ACTION_MOVE_RIGHT;

                if (input_pressed(state->game_input.keyboard.keys['r'])) state->actionState |= ACTION_RESTART;
                if (input_pressed(state->game_input.keyboard.keys['f'])) state->actionState |= ACTION_PICKUP;
                if (input_pressed(state->game_input.keyboard.keys['e'])) state->actionState |= ACTION_ATTACK;
            }
            // TODO use fixed delta time
            Reset::update(dt, &state->isRewinding, &state->loopTime, &state->cmdIdx, state->actionState);

            /* test our immediate mode ui */
            ui_begin(&ui_ctx);

            // ui input update
            ui_ctx.mouse_pos      = {state->game_input.mouse.pos.x,state->game_input.mouse.pos.y};
            ui_ctx.mouse_pressed  = input_pressed(state->game_input.mouse.buttons[0]);

            i32 btn_size_x = 100;
            i32 btn_size_y = 50;
            local u32 window_style = WINDOW_STYLE_VERTICAL;
            ui_window_begin(&ui_ctx, 50, 20, __COUNTER__, window_style);
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, &ui_entities[0].sprite, __COUNTER__))
                {
                    printf("pressed btn 1!\n");
                    state->entity_to_place = &ui_entities[0];
                }
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y,  &ui_entities[1].sprite, __COUNTER__))
                {
                    printf("pressed btn 2!\n");
                    state->entity_to_place = &ui_entities[1];
                }
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__,"stop"))
                {
                    state->entity_to_place = nullptr;
                }
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__))
                {
                    printf("pressed btn 4!\n");
                }

                local f32 r_value = 0.5f;
                local f32 g_value = 0.5f;
                local f32 b_value = 0.5f;
                ui_ctx.style.colors[UI_COLOR_BUTTON] = {r_value, g_value, b_value, 1.0f};
                if (ui_slider_float(&ui_ctx, &r_value, __COUNTER__) ||
                    ui_slider_float(&ui_ctx, &g_value, __COUNTER__) ||
                    ui_slider_float(&ui_ctx, &b_value, __COUNTER__))
                {
                    printf("new rgb: %.2f %.2f %.2f\n", r_value, g_value, b_value);
                }

                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__, "text\nwith\nnewl")) {}
                local b32 show_btn = false;
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__, "add a button"))
                {
                    show_btn = !show_btn;
                }
                local b32 show_window = false;
                if (show_btn && ui_button(&ui_ctx, btn_size_x+30, btn_size_y-20, NULL, __COUNTER__, "add a window"))
                {
                    show_window = !show_window;
                }
                if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__, "switch style"))
                {
                    window_style = window_style == 0 ? WINDOW_STYLE_VERTICAL : WINDOW_STYLE_HORIZONTAL;
                }
            ui_window_end(&ui_ctx);

            if (show_window)
            {
                ui_window_begin(&ui_ctx, 100, 700, __COUNTER__, WINDOW_STYLE_VERTICAL);
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_x, &state->tiles[5420].sprite, __COUNTER__))
                    {
                        printf("pressed button 1 in window 2!\n");
                    }
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_x, &state->tiles[4899].sprite, __COUNTER__))
                    {
                        printf("pressed button 2 in window 2!\n");
                    }
                ui_window_end(&ui_ctx);
            }

            ui_end(&ui_ctx);

            // TODO find out if it matters if we do everything in one loop for one
            // entity vs. every "system" has its own loop
            auto tiles = state->tiles;
            for (u32 i = 0; i < MAX_ENTITIES; i++)
            {
                auto& ent = state->ents[i];

                // PLAYER CONTROLLER ///////////////////////////////////////////////////////////////////////
                if (!state->isRewinding && ent.active)
                {
                    if (ent.flags & ENT_FLAG_PLAYER_CONTROLLED)
                    {
                        player_update(dt, ent, state->actionState, &state->cmdIdx);
                    }
                }

                // COMMAND REPLAY //////////////////////////////////////////////////////////////////////////
                if (!state->isRewinding && ent.active)
                {
                    if (ent.flags & ENT_FLAG_CMD_CONTROLLED)
                    {
                        command_record_or_replay(&ent, nullptr, &state->cmdIdx);
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
                        for (u32 k = 0; k < state->tile_count; k++)
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
                    Rewind::update(dt, ent, state->loopTime, state->isRewinding);
                }

                // NOTE animation should probably be last after input & collision etc.
                // TODO animation can crash if IS_ANIMATED entities don't have filled arrays..
                if (ent.flags & ENT_FLAG_IS_ANIMATED)
                {
                    ent.sprite.box = animation_update(&ent.anim, ent.clips, ent.clip_count, dt);
                }
            }

            // after loop update
            state->cmdIdx++; // command_on_update_end();
        } // update loop
    }

    state->cycles_left_over = update_iterations;
    state->last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////////////////////////////
    platform.renderer.push_clear({});
    { // layer_game_render();
        Entity* ents = state->ents;
        Tile* tiles  = state->tiles;

        // RENDER TILES ////////////////////////////////////////////////////////////////////////////
        for (u32 i = 0; i < state->tile_count; i++) // TODO tilemap culling
        {
            platform.renderer.push_sprite(tiles[i].sprite.tex, tiles[i].sprite.box,
                                          camera_world_to_screen(state->cam, tiles[i].position),
                                          state->cam.scale);
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

    ui_render(&ui_ctx, &platform);

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
