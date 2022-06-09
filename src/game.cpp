#include "game.hpp"
#include "game.h"

// TODO move this somewhere else ///////////////////////////////////////////////////////////////////
struct platform_window_t;
struct game_state_t
{
    b32 game_running = true;           // used by game NOTE: needs to be first
    b32 initialized = true;            // used by game
    platform_window_t* window;         // used by game, layer

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
    i32     focusedEntityIdx = -1;
    rect_t focusArrow = {224,96,16,32}; // used by layer TODO hardcoded
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

u32 SCREEN_WIDTH  = 1280;
u32 SCREEN_HEIGHT = 960;
const char* GAME_LEVEL = "res/map_level01.json";
const int MAX_RENDER_LAYERS = 100;

#include "interface.h"
ui_t ui_ctx = {};
Entity ui_entities[9] = {};

// TESTING AN IMMEDIATE-MODE ANIMATION API //////////////////////////////////////////////////////////////////////
struct animated_icon { f32 timer; animation_t anim; };
animated_icon icon_skele     = {0, {4, 0,   32, 16}}; // skeleton animation
animated_icon icon_necro     = {0, {4, 64,  32, 16}}; // necromancer animation
animated_icon icon_hourglass = {0, {6, 64, 208, 16}};
//////////////////////////////////////////////////////////////////////////////////////////////////////

global_var f32 zoom = 1.0f;;

global_var renderer_api_t renderer;

// TODO pass in game_input_t too (?)
extern "C" EXPORT void game_main_loop(game_state_t* state, platform_api_t platform)
{
    // INIT ///////////////////////////////////////////////////////////////////////////////
    if (!state->initialized)
    {
        state = new (state) game_state_t();
        platform.init("hello game", SCREEN_WIDTH, SCREEN_HEIGHT, &state->window, &renderer);
        state->game_input.window_width  = SCREEN_WIDTH;
        state->game_input.window_height = SCREEN_HEIGHT;
        { // layer_game_init:
            if (!levelgen_level_load(GAME_LEVEL, MAX_ENTITIES, &platform, &renderer, state)) exit(1);
            physics_init();
        }

        ui_init(&ui_ctx, &renderer);
        ui_entities[0] = create_entity_from_file("skeleton.ent", &renderer);
        ui_entities[1] = create_entity_from_file("necromancer.ent", &renderer);
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
        platform.event_loop(&state->game_input, state->window);
        SCREEN_WIDTH  = state->game_input.window_width;
        SCREEN_HEIGHT = state->game_input.window_height;

        // quit game on escape
        if (input_pressed(state->game_input.keyboard.keys['\e'])) state->game_running = false;

        { // game event handling
            v3i mouse_pos   = state->game_input.mouse.pos;
            v3f click       = camera_screen_to_world(state->cam, { (f32) mouse_pos.x, (f32) mouse_pos.y, 0 });
            v2i clickpoint  = {(i32) click.x, (i32) click.y};
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

            if (input_pressed(state->game_input.mouse.buttons[MOUSE_BUTTON_LEFT])
                && !ui_ctx.window_focused) // TODO hack
            {

                // get 'clicked on' playable entity
                b32 collided = false;
                for (u32 i = 0; i < MAX_ENTITIES; i++)
                {
                    auto ents = state->ents;
                    if (!ents[i].active) continue;
                    if (!(ents[i].flags & ENT_FLAG_CMD_CONTROLLED)) continue;
                    rect_t   coll       = ents[i].getColliderInWorld();

                    if (utils_point_in_rect(clickpoint, coll)) // TODO
                    {
                        collided = true;
                        if (state->focusedEntity)
                        {
                            state->focusedEntity->flags ^= ENT_FLAG_PLAYER_CONTROLLED;
                            state->focusedEntity->flags |= ENT_FLAG_CMD_CONTROLLED;
                        }
                        ents[i].flags |= ENT_FLAG_PLAYER_CONTROLLED;
                        ents[i].flags ^= ENT_FLAG_CMD_CONTROLLED;

                        state->focusedEntity    = &ents[i];
                        state->focusedEntityIdx = i;
                        break;
                    }
                }
                if (!collided) // unfocus
                {
                    if (state->focusedEntity)
                    {
                        state->focusedEntity->flags ^= ENT_FLAG_PLAYER_CONTROLLED;
                        state->focusedEntity->flags |= ENT_FLAG_CMD_CONTROLLED;
                    }
                    state->focusedEntity    = nullptr;
                    state->focusedEntityIdx = -1;
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
            // TODO lock zooming while lerping
            if (state->game_input.mouse.wheel > 0 || input_pressed(state->game_input.keyboard.keys['z']))
            {
                camera_zoom(state->cam, 0.5f, clickpoint);
            }
            if (state->game_input.mouse.wheel < 0 || input_pressed(state->game_input.keyboard.keys['x']))
            {
                camera_zoom(state->cam, 2.0f, clickpoint);
            }
        }

        { // camera zoom lerp
            Camera* cam = &state->cam;
            if (cam->rect.w != cam->target.w && cam->rect.h != cam->target.h)
            {
                cam->timer += dt;
                if (cam->timer > cam->TIME_TO_LERP)
                {
                    cam->timer = cam->TIME_TO_LERP;
                }
                cam->rect.w    = LERP(cam->source.w, cam->target.w, cam->timer/cam->TIME_TO_LERP);
                cam->rect.h    = LERP(cam->source.h, cam->target.h, cam->timer/cam->TIME_TO_LERP);
                cam->rect.left = LERP(cam->source.left, cam->target.left, cam->timer/cam->TIME_TO_LERP);
                cam->rect.top  = LERP(cam->source.top, cam->target.top, cam->timer/cam->TIME_TO_LERP);

                zoom = ( ((f32) cam->rect.w) / ((f32) SCREEN_WIDTH) );
            }
            else
            {
                cam->timer = 0;
            }
        }

        // UPDATE LOOP /////////////////////////////////////////////////////////////////////////////
        { // layer_game_update(dt);
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
            // NOTE break code hotloading right now, probably because the ui_context isn't in the game_state
            if(1){
                ui_begin(&ui_ctx);

                // ui input update
                ui_ctx.mouse_pos      = {state->game_input.mouse.pos.x,state->game_input.mouse.pos.y};
                ui_ctx.mouse_pressed  = input_pressed(state->game_input.mouse.buttons[0]);
                ui_ctx.cam            = state->cam;

                char string_buf_1[256];
                local sprite_t sprite = { {64, 208, 16, 16}, ui_entities[0].sprite.tex }; // TODO hardcoded
                sprintf(string_buf_1,"mouse x: %4d\nmouse y: %4d", ui_ctx.mouse_pos.x, ui_ctx.mouse_pos.y);
                char string_buf_2[256];
                sprintf(string_buf_2,"TICKS %d", platform.ticks());
                anim_update(&icon_hourglass.timer, &sprite.box, icon_hourglass.anim, dt);
                char string_buf_3[20];
                sprintf(string_buf_3,"Entity selected %d", state->focusedEntityIdx);
                ui_window_begin(&ui_ctx, 5, 5, __COUNTER__, WINDOW_LAYOUT_VERTICAL);
                    if (ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "X")) state->game_running = false;
                    ui_text(&ui_ctx, __COUNTER__, "Text here", 2);
                    ui_text(&ui_ctx, __COUNTER__, string_buf_1, 2);
                    ui_text(&ui_ctx, __COUNTER__, string_buf_2, 2);
                    ui_text(&ui_ctx, __COUNTER__, string_buf_3, 2);
                ui_window_end(&ui_ctx);
                ui_window_begin(&ui_ctx, SCREEN_WIDTH-100, 5, __COUNTER__, WINDOW_LAYOUT_VERTICAL);
                    ui_icon(&ui_ctx, __COUNTER__, sprite, 3.0f);
                ui_window_end(&ui_ctx);

                if (state->focusedEntityIdx != -1)
                {
                    ui_window_begin(&ui_ctx, SCREEN_WIDTH-200, 200, __COUNTER__, WINDOW_LAYOUT_HORIZONTAL);
                        ui_text(&ui_ctx, __COUNTER__, "ORIENT", 2);
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "UP"))
                            state->ents[state->focusedEntityIdx].orient = ENT_ORIENT_UP;
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "DOWN"))
                            state->ents[state->focusedEntityIdx].orient = ENT_ORIENT_DOWN;
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "LEFT"))
                            state->ents[state->focusedEntityIdx].orient = ENT_ORIENT_LEFT;
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "RIGHT"))
                            state->ents[state->focusedEntityIdx].orient = ENT_ORIENT_RIGHT;

                        ui_text(&ui_ctx, __COUNTER__, "STATE", 2);
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "MOVE"))
                            state->ents[state->focusedEntityIdx].state = ENT_STATE_MOVE;
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "HOLD"))
                            state->ents[state->focusedEntityIdx].state = ENT_STATE_HOLD;
                        if (ui_button(&ui_ctx, 100, 20, nullptr, __COUNTER__, "ATTACK"))
                            state->ents[state->focusedEntityIdx].state = ENT_STATE_ATTACK;
                    ui_window_end(&ui_ctx);
                }

                i32 btn_size_x = 100;
                i32 btn_size_y = 50;
                local u32 window_style = WINDOW_LAYOUT_VERTICAL;

                // animation test
                anim_update(&icon_skele.timer, &ui_entities[0].sprite.box, icon_skele.anim, dt);
                anim_update(&icon_necro.timer, &ui_entities[1].sprite.box, icon_necro.anim, dt);

                // proof-of-concept "scrolling"
                local const int NR_OF_BUTTONS = 6;
                local const int  nr_of_buttons_visible_at_a_time = 3;
                local bool button_visible[NR_OF_BUTTONS] = {false, true, true, true, false, false};
                local bool scroll_window = true;
                ui_window_begin(&ui_ctx, 850, 10, __COUNTER__);
                    if (ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "<")) // scroll left button
                    {
                        int btn_idx = 0;
                        while (!button_visible[btn_idx]) btn_idx++;

                        if (btn_idx != 0) // no wraparound
                        {
                            button_visible[btn_idx - 1] = true;
                            button_visible[(btn_idx-1) + nr_of_buttons_visible_at_a_time] = false;
                        }
                    }
                    if (button_visible[0] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "0")) {}
                    if (button_visible[1] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "1")) {}
                    if (button_visible[2] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "2")) {}
                    if (button_visible[3] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "3")) {}
                    if (button_visible[4] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "4")) {}
                    if (button_visible[5] && ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, "5")) {}
                    if (ui_button(&ui_ctx, 30, 30, nullptr, __COUNTER__, ">")) // scroll right button
                    {
                        int btn_idx = NR_OF_BUTTONS-1;
                        while (!button_visible[btn_idx]) btn_idx--;

                        if (btn_idx != (NR_OF_BUTTONS-1)) // no wraparound
                        {
                            button_visible[btn_idx+1] = true;
                            button_visible[(btn_idx+1) - nr_of_buttons_visible_at_a_time] = false;
                        }
                    }
                ui_window_end(&ui_ctx);

                ui_window_begin(&ui_ctx, 50, 800, __COUNTER__, window_style);
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_y, &ui_entities[0].sprite, __COUNTER__))
                    {
                        state->entity_to_place = &ui_entities[0];
                    }
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_y,  &ui_entities[1].sprite, __COUNTER__))
                    {
                        state->entity_to_place = &ui_entities[1];
                    }
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__,"stop"))
                    {
                        state->entity_to_place = nullptr;
                    }
                    if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__, "state machine"))
                    {
                        extern bool apply_state_machine;
                        apply_state_machine = !apply_state_machine;
                    }

                    local f32 r_value = 0.5f;
                    local f32 g_value = 0.5f;
                    local f32 b_value = 0.5f;
                    ui_ctx.style.colors[UI_COLOR_BUTTON] = {r_value, g_value, b_value, 1.0f};
                    if (ui_slider_float(&ui_ctx, &r_value, __COUNTER__) | // bitwise or to avoid short circuit
                        ui_slider_float(&ui_ctx, &g_value, __COUNTER__) | // evaluation
                        ui_slider_float(&ui_ctx, &b_value, __COUNTER__))
                    {
                        printf("new rgb: %.2f %.2f %.2f\n", r_value, g_value, b_value);
                    }

                    if (ui_button(&ui_ctx, btn_size_x, btn_size_y, NULL, __COUNTER__, "text with\nnewline chars")) {}
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
                        if (window_style == WINDOW_LAYOUT_VERTICAL)
                            window_style = WINDOW_LAYOUT_HORIZONTAL_UP;
                        else if (window_style == WINDOW_LAYOUT_HORIZONTAL_UP)
                            window_style = WINDOW_LAYOUT_VERTICAL;
                    }
                ui_window_end(&ui_ctx);

                if (show_window)
                {
                    ui_window_begin(&ui_ctx, 100, 700, __COUNTER__, WINDOW_LAYOUT_VERTICAL);
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
            }

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
                    if (ent.state != ENT_STATE_IDLE) // TODO workaround because rn we don't have idle anims
                    {
                        anim_update(&ent.anim_timer, &ent.sprite.box,
                                    ent.anims[ent.orient + (ent.state * ENT_ORIENT_COUNT)], dt);
                    }
                }
            }

            // after loop update
            state->cmdIdx++; // command_on_update_end();
        } // update loop
    }

    state->cycles_left_over = update_iterations;
    state->last_frame_time  = curr_time;

    // RENDERING ///////////////////////////////////////////////////////////////////////////////////
    { // layer_game_render();
        Entity* ents = state->ents;
        Tile* tiles  = state->tiles;

        // orthographic camera matrix 6 tuple:
        // (left,          right,                      bottom,                    top,          near, far)
        // (cam.rect.left, cam.rect.left + cam.rect.w, cam.rect.top + cam.rect.h, cam.rect.top,  -50,  50)
        f32 left   = (f32) state->cam.rect.left;
        f32 right  = (f32) state->cam.rect.left + state->cam.rect.w;
        f32 bottom = (f32) state->cam.rect.top  + state->cam.rect.h;
        f32 top    = (f32) state->cam.rect.top;
        f32 near   = -50;
        f32 far    = 50;
        cam_mtx_t cam_mtx = {0};
        //f32 matrix[4][4] = {{1,0,0,0},
        //                    {0,1,0,0},
        //                    {0,0,1,0},
        //                    {0,0,0,1}};
        //f32 matrix[4][4] = { {2/(right-left),              0,               0, -((right+left)/(right-left))},
        //                     {             0, 2/(top-bottom),               0, -((top+bottom)/(top-bottom))},
        //                     {             0,              0, (-2)/(far-near),     -((far+near)/(far-near))},
        //                     {             0,              0,               0,                            1}};
        f32 matrix[4][4] = { {2/(right-left),              0,               0, 0},
                             {             0, 2/(top-bottom),               0, 0},
                             {             0,              0, (2)/(far-near), 0},
                             {-((right+left)/(right-left)),-((top+bottom)/(top-bottom)), -((far+near)/(far-near)), 1}};
        //cam_mtx.mtx = matrix;
        memcpy(&cam_mtx.mtx, matrix, sizeof(f32) * 4 * 4);
        renderer.upload_camera(cam_mtx, zoom); // TODO should be Push... command

        // RENDER TILES ////////////////////////////////////////////////////////////////////////////
        for (u32 i = 0; i < state->tile_count; i++) // TODO tilemap culling
        {

            RENDERER_PUSH_SPRITE(tiles[i].sprite.tex, tiles[i].sprite.box,
                                          //camera_world_to_screen(state->cam,tiles[i].position),
                                          tiles[i].position,
                                          state->cam.scale);
        }

        // RENDER ENTITIES /////////////////////////////////////////////////////////////////////////
        for (u32 i = 0; i < MAX_ENTITIES; i++)
        {
            if (!ents[i].active) continue;
            if (ents[i].sprite.tex != nullptr) // workaround for temporary invisible entities,
                                               // TODO instead we should maybe just have an ENT_FLAG_RENDERED
                                               // that we check here
                RENDERER_PUSH_SPRITE(ents[i].sprite.tex, ents[i].sprite.box,
                                     //camera_world_to_screen(state->cam, ents[i].position),
                                     ents[i].position,
                                     state->cam.scale);
        }

        // draw focusarrow on focused entity TODO hardcoded & very hacky
        if (state->focusedEntity)
        {
            sprite_t arrow_sprite = { state->focusArrow, ents[0].sprite.tex };
            //auto pos = camera_world_to_screen(state->cam, state->focusedEntity->position);
            auto pos = state->focusedEntity->position;
            RENDERER_PUSH_SPRITE(arrow_sprite.tex, arrow_sprite.box, pos, state->cam.scale);

            // testing drawing colliders
            auto collider      = state->focusedEntity->getColliderInWorld();
            v3i pos_on_screen = {collider.left, collider.top, 0};
            rect_t collider_on_screen = { (i32)pos_on_screen.x, (i32)pos_on_screen.y, collider.w, collider.h };
            render_entry_rect_t rect = { collider_on_screen, -10, {1,1,1,1} };
            RENDERER_PUSH_RECT_OUTLINE(rect, 1);
        }

        if (state->entity_to_place)
        {
            RENDERER_PUSH_SPRITE(state->entity_to_place->sprite.tex, state->entity_to_place->sprite.box,
                                 state->entity_to_place->position, state->entity_to_place->scale);
        }
    }

    ui_render(&ui_ctx, &renderer, state->cam);

    renderer.render(state->window);

    // game_quit:
    if (!state->game_running)
    {
        platform.quit(state->window);
        exit(0);
    }
}
