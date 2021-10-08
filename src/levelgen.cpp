#include "layer.h"
#include "entity.h"
#include "resourcemgr.h"
#include "rewind.h"

// TODO use json files for levelgen
void json_array_traversal(struct json_array_s* array);
void json_object_traversal(struct json_object_s* object);
b32 levelgen_level_load(const std::string& file, Entity* ents, u32 max_ents,
                        game_state_t* game_state)

{
    { // testing json loading & parsing

        file_t file = platform.file_load("res/tiletest.json");

        // get the root of the json DOM
        // NOTE uses malloc, but can given an alloc_fun_ptr w/ user_data
        struct json_parse_result_s result;
        struct json_value_s* json_dom = json_parse_ex(file.buffer, file.size, 0,
                                                      NULL, NULL, &result);
        // close file & free buffer
        platform.file_close(file);

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
                    newEnt.sprite.tex   = resourcemgr_texture_load(ts->getImagePath().c_str(), game_state);
                    newEnt.renderLayer  = 1;
                    newEnt.orient       = ORIENT_DOWN;
                    const auto& aabb    = o.getAABB();
                    newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                           (i32) aabb.width, (i32) aabb.height};
                    newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;
                    //newEnt.flags       |= (u32) EntityFlag::PLAYER_CONTROLLED;
                    newEnt.flags       |= (u32) EntityFlag::CMD_CONTROLLED;
                    newEnt.flags       |= (u32) EntityFlag::IS_REWINDABLE;
                    Rewind::initializeFrames(newEnt);
                    CommandProcessor::initialize(newEnt);

                } else if (type == "Item") {
                    newEnt.sprite.box   = spritebox;
                    newEnt.sprite.pivot = {0.5f, 0.75f};
                    // TODO why -24
                    newEnt.setPivPos( {o.getPosition().x,
                                       o.getPosition().y - 24, 0});
                    newEnt.sprite.tex   = resourcemgr_texture_load(ts->getImagePath().c_str(), game_state);
                    const auto& aabb    = o.getAABB();
                    newEnt.collider     = {/*(i32) aabb.left,  (i32) aabb.top,*/ 0, 0,
                                           (i32) aabb.width, (i32) aabb.height};
                    newEnt.flags       |= (u32) EntityFlag::IS_COLLIDER;
                    newEnt.flags       |= (u32) EntityFlag::IS_ITEM;
                    newEnt.flags       |= (u32) EntityFlag::IS_REWINDABLE;
                    Rewind::initializeFrames(newEnt);
                }

                // copy new entity into array TODO slow
                EntityMgr::copyEntity(newEnt);
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
                newTile.sprite.tex   = resourcemgr_texture_load(ts->getImagePath().c_str(), game_state);
                newTile.setPivPos({x * 16.f, y * 16.f, 0});

                // copy new tile into array TODO slow
                EntityMgr::createTile(newTile);
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
