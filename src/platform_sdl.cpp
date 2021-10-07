#if defined(PLATFORM_SDL)

// NOTE SDL headers come from pch
#include "base.h"
#include "platform.h"
#include "input.h"

// UNITY BUILD
#include "platform_levelgen.cpp"

struct platform_window_t
{
    SDL_Window*   handle;
    SDL_Renderer* renderer;
};

#define SDL_ERROR(x) if (!x) { printf("SDL ERROR: %s\n", SDL_GetError()); }

#include "entity.h" // needed for sprite struct, TODO remove
#include "memory.h" // TODO avoid including this
game_state_t game_state = {};

// game functions
typedef void (*game_main_loop_fn)();
typedef b32  (*game_init_fn)(game_state_t*);
typedef void (*game_state_update_fn)(game_state_t*);
typedef b32  (*game_quit_fn)();
struct game_api_t
{
    game_state_update_fn   state_update;
    game_init_fn           init;
    game_main_loop_fn      main_loop;
    game_quit_fn           quit;
    int                    id;
};
static game_api_t game;

void platform_quit();
extern platform_api_t platform_api;
static b32 game_running = true;

#include <dlfcn.h>    // for opening shared objects (needs to be linked with -ldl)
#include <sys/stat.h> // for checking if dll changed on disk (TODO does it work crossplatform?)
static void* dll_handle = nullptr;
#ifdef PLATFORM_WIN32
  const char* GAME_DLL = "./dep/libgame.dll";
#else
  const char* GAME_DLL = "./dep/libgame.so";
#endif

// NOTE sdl also offers functions for dll loading, which might be crossplatform
// SDL_UnloadObject(), SDL_LoadObject(), SDL_LoadFunction()
// TODO use a (custom ?) error function and not printf
b32 platform_load_code()
{
    // unload old dll
    if (dll_handle)
    {
        game.state_update = nullptr;
        game.main_loop    = nullptr;
        game.init         = nullptr;
        game.quit         = nullptr;
        game.id           = 0;

        if (dlclose(dll_handle) != 0) printf("FAILED TO CLOSE DLL\n");
        dll_handle = nullptr;
    }

    // See https://nullprogram.com/blog/2014/12/23/
    // "It’s critically important that dlclose() happens before dlopen(). On my
    // system, dlopen() looks only at the string it’s given, not the file behind
    // it. Even though the file has been replaced on the filesystem, dlopen()
    // will see that the string matches a library already opened and return a
    // pointer to the old library. (Is this a bug?)"

    // NOTE try opening until it works, otherwise we need to sleep() for a moment to avoid a crash
    while (dll_handle == nullptr)
    {
        dll_handle = dlopen(GAME_DLL, RTLD_NOW);
        if (dll_handle == nullptr) printf("OPENING GAME DLL FAILED. TRYING AGAIN.\n");
    }

    if (dll_handle == nullptr)
    {
        printf("OPENING LIBGAME.SO FAILED\n");
        return false;
    }

    // TODO pass memory to new dll
    // ...

    game.state_update = (game_state_update_fn) dlsym(dll_handle, "game_state_update");
    game.main_loop    = (game_main_loop_fn)    dlsym(dll_handle, "game_main_loop");
    game.init         = (game_init_fn)         dlsym(dll_handle, "game_init");
    game.quit         = (game_quit_fn)         dlsym(dll_handle, "game_quit");

    if (!game.main_loop)
    {
        printf("FINDING GAME_MAIN FAILED\n");
        return false;
    }

    return true;
}

// entry point
int main(int argc, char* args[])
{
    //platform_init();

    // NOTE eventually we should malloc the game memory, but then we would have
    // to write out all init values here or use C++'s new keyword (so that the
    // default values written in the struct definitions are used)
    //game_state = (game_state_t*) malloc(sizeof(game_state_t));
    //memset(game_state, 0, sizeof(game_state_t));

    platform_load_code(); // initial loading of the game dll
    game_state.platform = platform_api;
    game.state_update(&game_state);
    game.init(&game_state);
    game_running = true;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(game_main_loop, -1, 1); // NOTE no code reloading
#else
    while (game_running)
    {
       game.main_loop();

       // TODO check if dll/so changed on disk
       // NOTE should only happen in debug builds
       struct stat attr;
       if ((stat(GAME_DLL, &attr) == 0) && (game.id != attr.st_ino))
       {
           printf("Attempting code reload\n");
           platform_load_code();
           game.id = attr.st_ino;
           game.state_update(&game_state); // pass memory to game dll
       }
    }
#endif

    game.quit();
    platform_quit();
}

platform_window_t* platform_window_open(const char* title, u32 screen_width, u32 screen_height)
{
    if (SDL_Init(SDL_INIT_TIMER
                 | SDL_INIT_AUDIO
                 | SDL_INIT_VIDEO
                 // | SDL_INIT_JOYSTICK
                 // | SDL_INIT_HAPTIC
                 // | SDL_INIT_GAMECONTROLLER
                 // | SDL_INIT_EVENTS
                 // | SDL_INIT_SENSOR
                 // | SDL_INIT_NOPARACHUTE
                 // | SDL_INIT_EVERYTHING
                 ) != 0)
    {
        printf("SDL init failed: %s\n", SDL_GetError());
    }

    // TODO don't call malloc
    platform_window_t* window = (platform_window_t*) malloc(sizeof(platform_window_t));

    window->handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      screen_width, screen_height, 0);
    SDL_ERROR(window->handle);

    window->renderer = SDL_CreateRenderer(window->handle, -1,
                                          SDL_RENDERER_ACCELERATED
                                          //| SDL_RENDERER_PRESENTVSYNC
                                          );
    SDL_ERROR(window->renderer);

    //SDL_RenderSetLogicalSize(rw->renderer, 640, 480);
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    //printf("%s\n", SDL_GetHint("SDL_HINT_RENDER_SCALE_QUALITY"));

    SDL_RenderSetScale(window->renderer, 1.f, 1.f);

    return window;
}

void platform_window_close(platform_window_t* window)
{
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->handle);
}

void* platform_file_load(const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "r"); // TODO opens in utf-8?

    if(!file)
    {
        printf("Warning: Unable to open file! SDL Error: %s\n", SDL_GetError());
    }

    return file;
}

//void* platform_file_save(u8* file_name, u8* buffer);

// EVENTS //////////////////////////////////////////////////////////////////////////////////////////

// counts up button presses between last and next frame
// TODO check if this actually works, i.e. if you can press a button more than once between frames
static inline
void input_event_process(game_input_state_t* new_state, b32 is_down)
{
    if(new_state->is_down != is_down)
    {
        new_state->is_down = is_down;
        ++new_state->up_down_count;
    }
}

void platform_event_loop(game_input_t* input)
{
    // TODO remove hardcoded resetting of halftransitioncount
    for(int key_idx = 0; key_idx < 128; /* TODO hardcoded */ ++key_idx)
        input->keyboard.keys[key_idx].up_down_count = 0;
    for(int btn_idx = 0; btn_idx < MOUSE_BUTTON_COUNT; /* TODO hardcoded */ ++btn_idx)
        input->mouse.buttons[btn_idx].up_down_count = 0;
    for(int f_idx = 0; f_idx < 13; /* TODO hardcoded */ ++f_idx)
        input->keyboard.f_key_pressed[f_idx] = false;
    input->keyboard.key_up.up_down_count = 0;
    input->keyboard.key_down.up_down_count = 0;
    input->keyboard.key_left.up_down_count = 0;
    input->keyboard.key_right.up_down_count = 0;

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    SDL_Keycode keycode = sdl_event.key.keysym.sym;
                    b32 is_down         = (sdl_event.key.state == SDL_PRESSED);

                    if(sdl_event.key.repeat == 0)
                    {
                        if (is_down)
                        {
                            if((keycode >= SDLK_F1) && (keycode <= SDLK_F12))
                            {
                                input->keyboard.f_key_pressed[keycode - SDLK_F1 + 1] = true;
                            }
                        }

                        if (keycode == SDLK_UP)    input_event_process(&input->keyboard.key_up, is_down);
                        if (keycode == SDLK_DOWN)  input_event_process(&input->keyboard.key_down, is_down);
                        if (keycode == SDLK_LEFT)  input_event_process(&input->keyboard.key_left, is_down);
                        if (keycode == SDLK_RIGHT) input_event_process(&input->keyboard.key_right, is_down);

                        // NOTE SDL Keycodes (SDLK_*) seem to map to ascii for a-z
                        // but other characters (e.g. winkey/f-keys) go over 128,
                        // so we mask off bits here
                        keycode &= 255;
                        input_event_process(&input->keyboard.keys[keycode], is_down);
                    }

                } break;

                case SDL_MOUSEMOTION:
                {
                    input->mouse.pos = {sdl_event.motion.x, sdl_event.motion.y, 0};
                } break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    auto button = sdl_event.button.button;
                    b32 is_down = sdl_event.button.state;
                    if (button == SDL_BUTTON_LEFT)
                        input_event_process(&input->mouse.buttons[MOUSE_BUTTON_LEFT], is_down);
                } break;

                case SDL_QUIT: { /*input->quit_requested = true;*/ game_running = false; } break;
                case SDL_WINDOWEVENT: {
                    if (sdl_event.window.type == SDL_WINDOWEVENT_CLOSE)
                    {
                        //input->quit_requested = true;
                        game_running = false;
                    }
                } break;
        }
    }
}

#include "json.h" // TODO use json files for levelgen
void json_array_traversal(struct json_array_s* array);
void json_object_traversal(struct json_object_s* object);
void json_object_traversal(struct json_object_s* object)
{
    for (json_object_element_s* elem = object->start; elem != nullptr; elem = elem->next)
    {
        switch (elem->value->type)
        {
            case json_type_string:
            {
                printf("%s has string %s\n", elem->name->string,
                       ((json_string_s*) elem->value->payload)->string);
            } break;

            case json_type_number:
            {
                printf("%s has number %s\n", elem->name->string,
                       ((json_number_s*) elem->value->payload)->number);
            } break;

            case json_type_object:
            {
                json_object_s* obj = (json_object_s*) elem->value->payload;
                json_object_traversal(obj);
            } break;

            // TODO
            // traverse through the layers
            case json_type_array:
            {
                struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
                size_t arr_len = array->length;
                struct json_array_element_s* first_elem = array->start;
                printf("%s array has length %zu\n", elem->name->string, arr_len);
                json_array_traversal(array);
            } break;

            case json_type_true:
            {
                printf("%s is set to TRUE\n", elem->name->string);
            } break;
            case json_type_false:
            {
                printf("%s is set to FALSE\n", elem->name->string);
            } break;
            case json_type_null:
            {
                printf("%s is set to NULL\n", elem->name->string);
            } break;
        }
    }
}

void json_array_traversal(struct json_array_s* array)
{
    for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
    {
        switch (elem->value->type)
        {
            case json_type_string:
            {
                printf("%s has string %s\n", ((json_string_s*)elem->value->payload)->string,
                       ((json_string_s*) elem->value->payload)->string);
            } break;

            case json_type_number:
            {
                printf("%s has number %s\n", ((json_string_s*)elem->value->payload)->string,
                       ((json_number_s*) elem->value->payload)->number);
            } break;

            case json_type_object:
            {
                json_object_s* obj = (json_object_s*) elem->value->payload;
                json_object_traversal(obj);
            } break;

            // traverse through the layers
            case json_type_array:
            {
                struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
                size_t arr_len = array->length;
                struct json_array_element_s* first_elem = array->start;
                printf("%s array has length %zu\n", ((json_string_s*)elem->value->payload)->string, arr_len);
                json_array_traversal(array);
            } break;

            case json_type_true:
            {
                //printf("%s is set to TRUE\n", elem->name->string);
            } break;
            case json_type_false:
            {
                //printf("%s is set to FALSE\n", elem->name->string);
            } break;
            case json_type_null:
            {
                //printf("%s is set to NULL\n", elem->name->string);
            } break;
        }
    }
}

void create_map_from_json(struct json_object_s* root)
{
    // https://doc.mapeditor.org/en/stable/reference/json-map-format/

    // A MAP IN TILED IS MADE UP OUT OF:
    //
    // s32 compressionlevel
    // u32 height
    // b32 infinite
    // layer_t layers[] ...
    // u32 nextlayerid // TODO what is thiss
    // u32 nextobjectid
    // orientation_e orientation // orthogonal, isometric, ...
    // renderorder_e renderorder // right-down, ...
    // char* tiledversion // e.g. "1.7.2"
    // u32 tileheight
    // tileset_t tilesets[] // TODO actually just contains "firstgid" & "source" (path)
    // u32 tilewidth
    // char* version // TODO version of what?
    // u32 width

    // A TILELAYER IN TILED IS MADE UP OUT OF:
    // u32 data[] // IDs that map to a sprite from a tileset
    // u32 height
    // u32 id
    // char* name
    // f32 opacity
    // type_t type // tilelayer,
    // b32 visible
    // u32 width
    // u32 x
    // u32 y
    //
    // AN OBJECTLAYER IN TILED IS MADE UP OUT OF:
    // draworder_e draworder // topdown, ...
    // u32 id
    // char* name
    // object_t objects[]
    // f32 opacity
    // type_t type // tilelayer,
    // b32 visible
    // u32 x
    // u32 y

    // A TILESET IN TILED IS MADE UP OUT OF:
    //
    // u32 columns
    // char* image // path to imagefile
    // u32 imageheight
    // u32 imagewidth
    // u32 margin
    // char* name
    // u32 spacing
    // u32 tilecount
    // char* tiledversion
    // u32 tileheight
    // tile_t tiles[]
    // u32 tilewidth
    // type_t type // tileset, ...
    // char* version

    // A TILE IN TILED IS MADE UP OUT OF:
    // u32 id
    // objectgroup_t objectgroup
    //
    // AN OBJECTGROUP IN TILED IS MADE UP OUT OF:
    // draworder_e draworder // index, ...
    // u32 id
    // char* name
    // object_t object
    // f32 opacity
    // type_t type // objectgroup, ...
    // b32 visible
    // u32 x
    // u32 y
    //
    // AN OBJECT IN TILED IS MADE UP OUT OF:
    // u32 height
    // u32 id
    // char* name
    // f32 rotation
    // type_t type // TODO ...
    // b32 visible
    // u32 width
    // u32 x
    // u32 y
}

// TODO level generation shouldn't be in the platform layer. Once we remove the
// external dependencies, we should move this into gamecode that calls
// platform_file_open() and so on
typedef void* (*resourcemgr_texture_load_fn)(const char*, game_state_t*);
typedef void* (*resourcemgr_font_load_fn)(const char*, game_state_t*, i32);
typedef bool (*copyEntity_fn)(const Entity);
typedef bool (*createTile_fn)(const Tile tile);
typedef void (*initializeFrames_fn)(Entity& e);
typedef void (*initialize_fn)(Entity& ent);
b32 platform_level_load(const std::string& file, Entity* ents, u32 max_ents,
                        game_state_t* game_state, resourcemgr_texture_load_fn texture_load,
                        resourcemgr_font_load_fn font_load,
                        copyEntity_fn copyEntity, createTile_fn createTile,
                        initializeFrames_fn Rewind_initializeFrames,
                        initialize_fn CommandProcessor_initialize)
{
    {// testing json loading & parsing
        //void* json_file_buf = platform_file_load("res/tiletest.json");

        // NOTE loading in file in binary-mode. Will this cause problems on
        // other platforms w/ different newline characters?
        SDL_RWops* file = SDL_RWFromFile("res/tiletest.json", "r+b");
        if(!file) printf("Warning: Unable to open file! SDL Error: %s\n", SDL_GetError());

        size_t file_size = file->size(file); // size in bytes
        u8* file_buf = (u8*) malloc(file_size);
        SDL_RWread(file, file_buf, sizeof(u8), file_size);

        // get the root of the json DOM
        // NOTE uses malloc, but can given an alloc_fun_ptr w/ user_data
        struct json_parse_result_s result;
        struct json_value_s* json_dom = json_parse_ex(file_buf, file_size,
                                                      json_parse_flags_allow_json5,
                                                      NULL, NULL, &result);
        // close file & free buffer
        SDL_RWclose(file);
        free(file_buf);

        if (!json_dom)
        {
            printf("json didn't parse\n");
            printf("json error type: %zu\n", result.error);
            printf("json error at line: %zu\n", result.error_line_no);
        }

        struct json_object_s* object = (struct json_object_s*) json_dom->payload;

        // traverse the linked list
        // NOTE recursively calls object_traversal & array_traversal
        json_object_traversal(object);
    }

    tmx::Map map;
    if (!map.load(file)) { printf("map didnt load"); return false; }

    tmx::Map animMap; // only for chars for now
    if (!animMap.load("res/character_anims.tmx")) { printf("animmap didnt load"); exit(1); }

    const auto& tilecountXY  = map.getTileCount();
    //u32 max_tiles            = tilecountXY.x * tilecountXY.y;
    const auto& layers       = map.getLayers();
    u32 layercount           = 0;
    u32 tilecount            = 0;

    const auto& tilesets = map.getTilesets();
    const tmx::Tileset* ts = &tilesets.at(0);

    for(const auto& layer : layers)
    {
        tilecount = 0;

        // for items & characters
        if (layer->getType() == tmx::Layer::Type::Object)
        {
            const auto& objs = layer->getLayerAs<tmx::ObjectGroup>().getObjects();
            //auto& ts       = tilesets.at(1); //TODO hardcoded

            for (const auto& o : objs)
            {
                const std::string& type = o.getType();
                //const std::string& name = o.getName();
                Entity newEnt = {0};
                newEnt.active       = true;
                newEnt.freed        = false;
                newEnt.renderLayer  = layercount;

                // determine tileset
                for (int i = 0; i < tilesets.size(); i++)
                {
                    if (tilesets.at(i).hasTile(o.getTileID()))
                    {
                        ts = &tilesets.at(i);
                        break;
                    }
                }
                auto t = ts->getTile(o.getTileID());

                // to create the spritebox
                rect_t spritebox = {0};
                ASSERT(t != nullptr);
                spritebox = { (i32) t->imagePosition.x, (i32) t->imagePosition.y,
                              (i32) t->imageSize.x,     (i32) t->imageSize.y };

                // TODO load in anims in here
                // TODO entity_create_character()
                if (type == "Character")
                {

                    // load in animations TODO this only needs to be loaded in
                    // once & not per character
                    //u32 anim_idx = 0;
                    for (auto anim : animMap.getAnimatedTiles())
                    {
                        std::vector<AnimationFrame> new_frames;
                        //u32 frame_count = 0;
                        for (auto frame : anim.second.animation.frames)
                        {
                            auto tileID = frame.tileID;
                            // TODO get proper tileset
                            auto pos    = animMap.getTilesets().at(0).getTile(tileID)->imagePosition;
                            auto size   = animMap.getTilesets().at(0).getTile(tileID)->imageSize;
                            //auto pos    = ts->getTile(tileID)->imagePosition;
                            //auto size   = ts->getTile(tileID)->imageSize;
                            rect_t bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
                            new_frames.push_back({bb, (f32) frame.duration/100.f}); // TODO why cast?
                        }
                        newEnt.clips[newEnt.clip_count].frames = new_frames;
                        newEnt.clips[newEnt.clip_count].loop   = true;
                        newEnt.clip_count++;
                        newEnt.flags       |= (u32) EntityFlag::IS_ANIMATED;
                    }

                    // TODO charID
                    newEnt.sprite.box   = spritebox;
                    newEnt.sprite.pivot = {0.5f, 0.5f};
                    newEnt.state        = STATE_MOVE;
                    // TODO why -24
                    newEnt.setPivPos( {o.getPosition().x,
                                       o.getPosition().y - 24, 0});
                    // TODO platform code (?)
                    newEnt.sprite.tex   = texture_load(ts->getImagePath().c_str(), game_state);
                    newEnt.renderLayer  = 1;
                    newEnt.orient       = ORIENT_DOWN;
                    const auto& aabb    = o.getAABB();
                    newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                           (i32) aabb.width, (i32) aabb.height};
                    newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;
                    //newEnt.flags       |= (u32) EntityFlag::PLAYER_CONTROLLED;
                    newEnt.flags       |= (u32) EntityFlag::CMD_CONTROLLED;
                    newEnt.flags       |= (u32) EntityFlag::IS_REWINDABLE;
                    Rewind_initializeFrames(newEnt);
                    CommandProcessor_initialize(newEnt);

                } else if (type == "Item") {
                    newEnt.sprite.box   = spritebox;
                    newEnt.sprite.pivot = {0.5f, 0.75f};
                    // TODO why -24
                    newEnt.setPivPos( {o.getPosition().x,
                                       o.getPosition().y - 24, 0});
                    newEnt.sprite.tex   = texture_load(ts->getImagePath().c_str(), game_state);
                    const auto& aabb    = o.getAABB();
                    newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                           (i32) aabb.width, (i32) aabb.height};
                    newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;
                    newEnt.flags       |= (u32) EntityFlag::IS_ITEM;
                    newEnt.flags       |= (u32) EntityFlag::IS_REWINDABLE;
                    Rewind_initializeFrames(newEnt);
                }

                // copy new entity into array TODO slow
                copyEntity(newEnt);
            } // object loop
        } // objectlayer

        // for static tiles w/ and w/o colliders
        if(layer->getType() == tmx::Layer::Type::Tile)
        {
            const auto& tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();

            for (const auto& t : tiles)
            {
                // determine tilset
                for (int i = 0; i < tilesets.size(); i++)
                {
                    if (tilesets.at(i).hasTile(t.ID))
                    {
                        ts = &tilesets.at(i);
                        break;
                    }
                }

                u32 y = tilecount / tilecountXY.x;
                u32 x = tilecount % tilecountXY.y;

                auto tile = ts->getTile(t.ID);
                if (!tile) { tilecount++; continue; }
                rect_t bb = {(i32) tile->imagePosition.x,
                               (i32) tile->imagePosition.y,
                               (i32) tile->imageSize.x,
                               (i32) tile->imageSize.y};

                // CONSTRUCT TILE
                Tile newTile = {0};
                if (!tile->objectGroup.getObjects().empty())
                {
                    // TODO collision box data uses pixels as units, we
                    // might want to convert this to a 0-1 range
                    const auto& aabb = tile->objectGroup.getObjects().at(0).getAABB();
                    newTile.collider  = {(i32) aabb.left,  (i32) aabb.top,
                                        (i32) aabb.width, (i32) aabb.height};
                    newTile.collidable = true;
                }

                newTile.renderLayer  = layercount;
                newTile.sprite.box   = bb;
                newTile.sprite.pivot = {0.5f, 0.5f};
                newTile.sprite.tex   = texture_load(ts->getImagePath().c_str(), game_state);
                newTile.setPivPos({x * 16.f, y * 16.f, 0});

                // copy new tile into array TODO slow
                createTile(newTile);
                tilecount++;
            } // tile loop
        } // tilelayer
        layercount++;
    } // layer loop

    //// Loading in animations
    //tmx::Map charMap;
    //if (!charMap.load("res/character.tmx")) { printf("charmap didnt load"); exit(1); }

    //// testing loading animations from .tmx (/.tsx) files
    //u32 animIndex = 0;
    //Animation anims[STATE_COUNT * ORIENT_COUNT];
    //Entity* entts = EntityMgr::getArray();
    //for (auto anim : charMap.getAnimatedTiles())
    //{
    //    for (auto frame : anim.second.animation.frames)
    //    {
    //        auto tileID = frame.tileID;
    //        auto pos    = charMap.getTilesets().at(0).getTile(tileID)->imagePosition;
    //        auto size   = charMap.getTilesets().at(0).getTile(tileID)->imageSize;
    //        SDL_Rect bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
    //        anims[animIndex].frames.push_back(bb);
    //        anims[animIndex].loop = true;
    //        anims[animIndex].length = 1.0f;

    //    }
    //    animIndex++;
    //}
    //for (u32 i = 0; i < MAX_ENTITIES; i++)
    //{
    //    auto entts = EntityMgr::getArray();
    //    if ((entts[i].flags & (u32) EntityFlag::PLAYER_CONTROLLED))
    //    {
    //        memcpy(entts[i].anims, anims, sizeof(anims));
    //        entts[i].anim = entts[i].anims[0];
    //    }
    //}

    return true;
}

// TODO replace all calls to this with calls to platform_render_texture
void platform_render_sprite(platform_window_t* window, const sprite_t& spr,
                            v3f position, f32 scale, u32 flip_type)
{
   SDL_Rect dst = {(int) position.x, (int) position.y,
                   (i32) (scale * spr.box.w), (i32) (scale * spr.box.h)};

   // NOTE: flipping seems expensive, maybe just store flipped sprites in
   // the spritesheet & add dedicated animations for those
   SDL_RenderCopyEx(window->renderer, (SDL_Texture*) spr.tex, (SDL_Rect*) &spr.box,
                    &dst, 0, NULL, (SDL_RendererFlip) flip_type);
   //SDL_RenderCopy(renderer, spr.tex, &spr.box, &dst);

}

void platform_render_texture(platform_window_t* window, texture_t* texture, rect_t* src, rect_t* dst)
{
    SDL_RenderCopy(window->renderer, (SDL_Texture*) texture, (SDL_Rect*) src, (SDL_Rect*) dst);
}

void platform_render_clear(platform_window_t* window)
{
    SDL_RenderClear(window->renderer);
}

void platform_render_present(platform_window_t* window)
{
    SDL_RenderPresent(window->renderer);
}

void platform_render_set_draw_color(platform_window_t* window, u8 r, u8 g, u8 b, u8 a)
{
    SDL_SetRenderDrawColor(window->renderer, r, g, b, a);
}

void platform_debug_draw(platform_window_t* window, const Entity& e, v3f pos)
{
    SDL_Rect dst = {(int) pos.x + e.collider.x, (int) pos.y + e.collider.y,
                    (i32) (e.scale * e.collider.w), (i32) (e.scale * e.collider.h)};

    // don't draw 'empty' colliders (otherwise it will draw points & lines)
    if (!SDL_RectEmpty(&dst)) // if (!(dst.h <= 0.f && dst.w <= 0.f))
        SDL_RenderDrawRect(window->renderer, &dst);

    //SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)

    // TODO isn't where it's expected
    // Draw pivot point
    SDL_RenderDrawPointF(window->renderer, pos.x, pos.y);
}

void platform_debug_draw_rect(platform_window_t* window, rect_t* dst)
{
    SDL_RenderDrawRect(window->renderer, (SDL_Rect*) dst);
}

texture_t* platform_texture_create_from_surface(platform_window_t* window, surface_t* surface)
{
    SDL_Texture* tex = SDL_CreateTextureFromSurface(window->renderer, (SDL_Surface*) surface);
    SDL_ERROR(tex);
    return tex;
}

texture_t* platform_texture_load(platform_window_t* window, const char* filename)
{
    SDL_Texture* tex = IMG_LoadTexture(window->renderer, filename);
    SDL_ERROR(tex);
    return tex;
}

i32 platform_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    return SDL_QueryTexture((SDL_Texture*) tex, format, access, w, h);
}

i32 platform_texture_set_blend_mode(texture_t* tex, u32 mode)
{
    return SDL_SetTextureBlendMode((SDL_Texture*) tex, (SDL_BlendMode) mode);
}

i32 platform_texture_set_alpha_mod(texture_t* tex, u8 alpha)
{
    return SDL_SetTextureAlphaMod((SDL_Texture*) tex, alpha);
}

void platform_surface_destroy(surface_t* surface)
{
    SDL_FreeSurface((SDL_Surface*) surface);
}

// SDL TTF extension ///////////////////////////////////////////////////////////////////////////////
font_t* platform_font_load(const char* filename, i32 ptsize)
{
    TTF_Font* font = TTF_OpenFont(filename, ptsize);
    SDL_ERROR(font);
    return font;
}

void platform_font_init()
{
    TTF_Init();
}

// TODO pass options to render blended/wrapped
surface_t* platform_text_render(font_t* font, const char* text, color_t color, u32 wrap_len)
{
    SDL_Surface* text_surf = TTF_RenderText_Blended_Wrapped((TTF_Font*) font, text,
                                                            *((SDL_Color*) &color), wrap_len);
    SDL_ERROR(text_surf);
    return text_surf;
}

u32  platform_ticks() { return SDL_GetTicks(); }

void platform_quit()
{
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// IMGUI BACKEND ///////////////////////////////////////////////////////////////////////////////////
void platform_imgui_init(platform_window_t* window, u32 screen_width, u32 screen_height)
{
#ifdef IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiSDL::Initialize(window->renderer, screen_width, screen_height);
    // WORKAROUND: imgui_impl_sdl.cpp doesn't know the window (g_Window) if we
    // don't call an init function, but all of them require a rendering api
    // (InitForOpenGL() etc.). This breaks a bunch of stuff in the
    // eventhandling. We expose the internal function below to circumvent that.
    ImGui_ImplSDL2_Init(window->handle);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
#endif
}

void platform_imgui_destroy()
{
#ifdef IMGUI
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
#endif
}

void platform_imgui_event_handle(game_input_t* input)
{
#ifdef IMGUI
    // NOTE for some reason we don't have to call this...
    //ImGui_ImplSDL2_ProcessEvent(&e.sdl);

    ImGuiIO& io = ImGui::GetIO();

    // don't let mouse clicks on imgui propagate through underlying layers
    if (io.WantCaptureMouse)
    {
        // TODO hack: zero out mouseclick data so layers underneath don't react to them
        memset(input->mouse.buttons, 0, sizeof(input->mouse.buttons));
    }

    //io.WantCaptureKeyboard;

    //e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
    //e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
#endif
}

void platform_imgui_begin(platform_window_t* window)
{
#ifdef IMGUI
    ImGui_ImplSDL2_NewFrame(window->handle);
    ImGui::NewFrame();
#endif
}

void platform_imgui_end()
{
#ifdef IMGUI
    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());
#endif
}
#endif // PLATFORM_SDL

platform_api_t platform_api =
{
  &platform_file_load,
  &platform_level_load,
  &platform_window_open,
  &platform_window_close,
  &platform_event_loop,
  &platform_ticks,
  &platform_quit,
  &platform_render_sprite,
  &platform_render_texture,
  &platform_render_clear,
  &platform_render_present,
  &platform_render_set_draw_color,
  &platform_texture_create_from_surface,
  &platform_texture_load,
  &platform_texture_query,
  &platform_texture_set_blend_mode,
  &platform_texture_set_alpha_mod,
  &platform_surface_destroy,
  &platform_font_init,
  &platform_font_load,
  &platform_text_render,
  &platform_debug_draw,
  &platform_debug_draw_rect,
  &platform_imgui_init,
  &platform_imgui_destroy,
  &platform_imgui_event_handle,
  &platform_imgui_begin,
  &platform_imgui_end
};
