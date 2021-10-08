#include "json.h"
#include "layer.h"
#include "entity.h"
#include "resourcemgr.h"
#include "rewind.h"

#include "levelgen.h"

// TODO use json files for levelgen
void create_map_from_json(struct json_value_s* root, tiled_map_t* map);
void json_array_traversal(struct json_array_s* array, tiled_map_t* map);
void json_object_traversal(struct json_object_s* object, tiled_map_t* map);
b32 levelgen_level_load(const std::string& file, Entity* ents, u32 max_ents, game_state_t* game_state)
{
    // testing json loading & parsing
    file_t json_file = platform.file_load("res/tiletest.json");

    // get the root of the json DOM
    // NOTE uses malloc, but can given an alloc_fun_ptr w/ user_data
    struct json_parse_result_s result;
    struct json_value_s* json_dom = json_parse_ex(json_file.buffer, json_file.size, 0,
                                                  NULL, NULL, &result);
    // close file & free buffer
    platform.file_close(json_file);

    if (!json_dom)
    {
        printf("json didn't parse\n");
        printf("json error type: %zu\n", result.error);
        printf("json error at line: %zu\n", result.error_line_no);
    }

    tiled_map_t map = {};

    struct json_object_s* object = (struct json_object_s*) json_dom->payload;
    for (json_object_element_s* elem = object->start; elem != nullptr; elem = elem->next)
    {
        const char* name = elem->name->string;

        if (strcmp(name, "height") == 0) map.height = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "layers") == 0)
        {
            // array traversal
            struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
            size_t arr_len = array->length;
            for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
            {
                // create layer
                tiled_layer_t* layer = &map.layers[map.layer_count++];
                for (json_object_element_s* obj = ((json_object_element_s*) elem->value)->next;
                     obj != NULL; obj = obj->next)
                {
                    const char* name = obj->name->string;
                    if (strcmp(name, "data") == 0)
                    {
                        // fill layer->data array
                        struct json_array_s* array = ((struct json_array_s*) obj->value->payload);
                        u32 array_idx = 0;
                        for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
                        {
                            layer->data[array_idx++] = atoi(json_value_as_number(elem->value)->number);
                        }
                    }
                    else if (strcmp(name, "draworder") == 0)
                    {
                        const char* string = (json_value_as_string(obj->value)->string);
                        if (strcmp(string, "topdown") == 0) layer->draworder = false;
                        else if (strcmp(string, "index") == 0)   layer->draworder = true;
                        else UNREACHABLE("Unknown draworder\n");
                    }
                    else if (strcmp(name, "height") == 0)
                        layer->height = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "id") == 0)
                        layer->id = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "name") == 0)
                        layer->name = json_value_as_string(obj->value)->string; // TODO use sth. like strcpy()
                    else if (strcmp(name, "objects") == 0)
                    {
                        // TODO
                    }
                    else if (strcmp(name, "opacity") == 0)
                        layer->opacity = atof(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "type") == 0)
                    {
                        const char* string = (json_value_as_string(obj->value)->string);
                        if (strcmp(string, "tilelayer") == 0)        layer->type = TILED_LAYER_TYPE_TILELAYER;
                        else if (strcmp(string, "objectgroup") == 0) layer->type = TILED_LAYER_TYPE_OBJECTGROUP;
                        else if (strcmp(string, "imagelayer") == 0)  layer->type = TILED_LAYER_TYPE_IMAGELAYER;
                        else if (strcmp(string, "group") == 0)       layer->type = TILED_LAYER_TYPE_GROUP;
                        else UNREACHABLE("Unknown type for layer\n");
                    }
                    else if (strcmp(name, "visible") == 0) layer->visible = !json_value_is_false(obj->value);
                    else if (strcmp(name, "width") == 0) layer->width = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "x") == 0) layer->x = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "y") == 0) layer->y = atoi(json_value_as_number(obj->value)->number);
                    else UNREACHABLE("Unknown attribute.\n");
                }
            }
        }
        else if (strcmp(name, "orientation") == 0)
        {
            const char* orient = json_value_as_string(elem->value)->string;
            if (strcmp(orient, "orthogonal") == 0)     map.orientation = TILED_ORIENTATION_ORTHOGONAL;
            else if (strcmp(orient, "isometric") == 0) map.orientation = TILED_ORIENTATION_ISOMETRIC;
            else if (strcmp(orient, "staggered") == 0) map.orientation = TILED_ORIENTATION_STAGGERED;
            else if (strcmp(orient, "hexagonal") == 0) map.orientation = TILED_ORIENTATION_HEXAGONAL;
            else UNREACHABLE("unknown orientation for map\n");
        }
        else if (strcmp(name, "renderorder") == 0)
        {
            const char* order = json_value_as_string(elem->value)->string;
            if (strcmp(order, "right-down") == 0)     map.renderorder = TILED_RENDERORDER_RIGHT_DOWN;
            else if (strcmp(order, "right-up") == 0)  map.renderorder = TILED_RENDERORDER_RIGHT_UP;
            else if (strcmp(order, "left-down") == 0) map.renderorder = TILED_RENDERORDER_LEFT_DOWN;
            else if (strcmp(order, "left-up") == 0)   map.renderorder = TILED_RENDERORDER_LEFT_UP;
        }
        else if (strcmp(name, "tileheight") == 0)
            map.tileheight = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "tilesets") == 0)
        {
            // TODO
            // iterate through array

            // if (strcmp(name, "columns") == 0) {}
            //else if (strcmp(name, "firstgid") == 0) {}
            //else if (strcmp(name, "image") == 0) {}
        }
        else if (strcmp(name, "tilewidth") == 0)
            map.tilewidth = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "width") == 0)
            map.width = atoi(json_value_as_number(elem->value)->number);
    }

    //create_map_from_json(json_dom, &map);
    printf("LAYERCOUNT: %u\n", map.layer_count);
    printf("mapheight: %u\n", map.height);
    printf("mapwidth: %u\n", map.width);
    printf("tileheight: %u\n", map.tileheight);
    printf("tilewidth: %u\n", map.tilewidth);

    printf("data 0: %u\n", map.layers[0].data[0]);
    printf("data 1: %u\n", map.layers[0].data[1]);
    printf("data 2: %u\n", map.layers[0].data[2]);
    printf("layer 1 is visible: %u\n", map.layers[0].visible);
    printf("layer 1 width: %u\n", map.layers[0].width);
    printf("layer 1 opacity: %f\n", map.layers[0].opacity);

    //////////////////////////////////////////////////////////////////////////////////////////////

    {
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
    }

    return true;
}

void json_object_traversal(struct json_object_s* object, tiled_map_t* map)
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
                json_object_traversal(obj, map);
            } break;

            // TODO
            // traverse through the layers
            case json_type_array:
            {
                struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
                size_t arr_len = array->length;
                struct json_array_element_s* first_elem = array->start;
                printf("%s array has length %zu\n", elem->name->string, arr_len);

                if (strcmp(elem->name->string, "layers"))
                {
                    // create layers
                    // map->layers[layer_count]...
                }
                json_array_traversal(array, map);
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

void json_array_traversal(struct json_array_s* array, tiled_map_t* map)
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
                json_object_traversal(obj, map);
            } break;

            // traverse through the layers
            case json_type_array:
            {
                struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
                size_t arr_len = array->length;
                struct json_array_element_s* first_elem = array->start;
                printf("%s array has length %zu\n", ((json_string_s*)elem->value->payload)->string, arr_len);
                json_array_traversal(array, map);
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

void create_map_from_json(struct json_value_s* root, tiled_map_t* map)
{
    struct json_object_s* object = (struct json_object_s*) root->payload;
    // traverse the linked list
    // NOTE recursively calls object_traversal & array_traversal
    json_object_traversal(object, map);


}
