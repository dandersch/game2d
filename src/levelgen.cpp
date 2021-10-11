#include "layer.h"
#include "entity.h"
#include "resourcemgr.h"
#include "rewind.h"

#include "levelgen.h"

//internal
void copy_json_string(char* str_buf, u32 buf_size, const char* string)
{
    ASSERT(buf_size >= strlen(string));
    strcpy(str_buf, string);
}

//internal
void fill_objects_array(struct json_value_s* value, tiled_layer_t* layer)
{
    struct json_array_s* array = ((struct json_array_s*) value->payload);
    size_t arr_len = array->length;
    // NOTE we could dynamically allocate memory here
    for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
    {
        tiled_object_t* o = &layer->objects[layer->obj_count++];
        for (json_object_element_s* obj = ((json_object_element_s*) elem->value)->next; obj != NULL; obj = obj->next)
        {
            // TODO this is duplicated from above; pull into function
            const char* name = obj->name->string;
            if (strcmp(name, "gid") == 0) o->gid = atoi(json_value_as_number(obj->value)->number);
            else if (strcmp(name, "height") == 0) o->height = atof(json_value_as_number(obj->value)->number);
            else if (strcmp(name, "id") == 0) o->id = atoi(json_value_as_number(obj->value)->number);
            else if (strcmp(name, "name") == 0)
                copy_json_string(o->name, sizeof(o->name), json_value_as_string(obj->value)->string);
            else if (strcmp(name, "properties") == 0)
            {
                // TODO
            }
            else if (strcmp(name, "rotation") == 0)
            {
                // TODO
            }
            else if (strcmp(name, "type") == 0)
                copy_json_string(o->type, sizeof(o->type), json_value_as_string(obj->value)->string);
            else if (strcmp(name, "visible") == 0) o->visible = !json_value_is_false(obj->value);
            else if (strcmp(name, "width") == 0) o->width = atof(json_value_as_number(obj->value)->number);
            else if (strcmp(name, "x") == 0) o->x = atof(json_value_as_number(obj->value)->number);
            else if (strcmp(name, "y") == 0) o->y = atof(json_value_as_number(obj->value)->number);
            else UNREACHABLE("unknown attribute '%s' for tiled object\n", name);
        }
    }
}

#include "debug.h" // for rudimentary profiling
b32 levelgen_level_load(const char* file, Entity* ents, u32 max_ents, game_state_t* game_state)
{
    // TODO measure performance of json parsing vs xml parsing

    struct json_value_s* json_dom;
    { // JSON PARSING
        file_t json_file = platform.file_load("res/tiletest.json");

        // get the root of the json DOM
        // NOTE library uses malloc, but canb e given an alloc_fun_ptr w/ user_data
        struct json_parse_result_s result;
        json_dom = json_parse_ex(json_file.buffer, json_file.size, 0, NULL, NULL, &result);
        if (!json_dom) printf("JSON didn't parse. Error type '%zu' at line %zu\n",
                              result.error, result.error_line_no);
        platform.file_close(json_file); // close file & free buffer
    }

    // NOTE right now this needs to be malloc'ed, stack allocation causes a
    // crash since tiled_map_t is too large (to the point where zeroing out the
    // memory creates a noticeable delay). That is mostly due to the fact that
    // all arrays inside the struct (including strings) are statically
    // allocated. This should be changed.
    tiled_map_t* map;
    map = (tiled_map_t*) malloc(sizeof(tiled_map_t));
    // memset(map, 0, sizeof(*map));

    { TIMED_BLOCK(); // id 0, profile our json serialization
    struct json_object_s* object = (struct json_object_s*) json_dom->payload;

    // NOTE way to make this more performant w/ fewer calls to strcmp()
    // json_object_element_s* elem = object->start;
    // if (strcmp(name, "compressionlevel") == 0) { set variable; elem = elem->next; name = elem->name->string; }
    // if (strcmp(name, "height") == 0) { set variable; elem = elem->next; name = elem->name->string; }
    // if (strcmp(name, "infinite") == 0) { set variable; elem = elem->next; name = elem->name->string; }
    // if (strcmp(name, "layers") == 0) { ... }
    // ...
    // if (elem != nullptr) UNREACHABLE("Unknown attribute '%s' for _\n", name);

    for (json_object_element_s* elem = object->start; elem != nullptr; elem = elem->next)
    {
        const char* name = elem->name->string;

        if (strcmp(name, "compressionlevel") == 0)
            map->compressionlevel = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "height") == 0) map->height = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "infinite") == 0) map->infinite = !json_value_is_false(elem->value);
        else if (strcmp(name, "layers") == 0)
        {
            // array traversal
            struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
            size_t arr_len = array->length;
            // NOTE we could dynamically allocate memory for the layers array based on arr_len here
            for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
            {
                // create layer
                tiled_layer_t* layer = &map->layers[map->layer_count++];
                for (json_object_element_s* obj = ((json_object_element_s*) elem->value)->next;
                     obj != NULL; obj = obj->next)
                {
                    const char* name = obj->name->string;
                    if (strcmp(name, "data") == 0)
                    {
                        // fill layer->data array
                        struct json_array_s* array = ((struct json_array_s*) obj->value->payload);
                        for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
                        {
                            layer->data[layer->tile_count++] = atoi(json_value_as_number(elem->value)->number);
                        }
                    }
                    else if (strcmp(name, "draworder") == 0)
                    {
                        const char* string = (json_value_as_string(obj->value)->string);
                        if (strcmp(string, "topdown") == 0) layer->draworder = false;
                        else if (strcmp(string, "index") == 0)   layer->draworder = true;
                        else UNREACHABLE("Unknown draworder '%s'\n", name);
                    }
                    else if (strcmp(name, "height") == 0)
                        layer->height = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "id") == 0)
                        layer->id = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "name") == 0)
                        copy_json_string(layer->name, sizeof(layer->name), json_value_as_string(obj->value)->string);
                    else if (strcmp(name, "objects") == 0)
                    {
                        fill_objects_array(obj->value, layer);
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
                        else UNREACHABLE("Unknown type '%s' for layer\n", string);
                    }
                    else if (strcmp(name, "visible") == 0) layer->visible = !json_value_is_false(obj->value);
                    else if (strcmp(name, "width") == 0) layer->width = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "x") == 0) layer->x = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "y") == 0) layer->y = atoi(json_value_as_number(obj->value)->number);
                    else UNREACHABLE("Unknown attribute '%s' for layer\n", name);
                }
            }
        }
        else if (strcmp(name, "nextlayerid") == 0)
            map->nextlayerid = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "nextobjectid") == 0)
            map->nextobjectid = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "orientation") == 0)
        {
            const char* orient = json_value_as_string(elem->value)->string;
            if (strcmp(orient, "orthogonal") == 0)     map->orientation = TILED_ORIENTATION_ORTHOGONAL;
            else if (strcmp(orient, "isometric") == 0) map->orientation = TILED_ORIENTATION_ISOMETRIC;
            else if (strcmp(orient, "staggered") == 0) map->orientation = TILED_ORIENTATION_STAGGERED;
            else if (strcmp(orient, "hexagonal") == 0) map->orientation = TILED_ORIENTATION_HEXAGONAL;
            else UNREACHABLE("unknown orientation '%s' for map\n", orient);
        }
        else if (strcmp(name, "renderorder") == 0)
        {
            const char* order = json_value_as_string(elem->value)->string;
            if (strcmp(order, "right-down") == 0)     map->renderorder = TILED_RENDERORDER_RIGHT_DOWN;
            else if (strcmp(order, "right-up") == 0)  map->renderorder = TILED_RENDERORDER_RIGHT_UP;
            else if (strcmp(order, "left-down") == 0) map->renderorder = TILED_RENDERORDER_LEFT_DOWN;
            else if (strcmp(order, "left-up") == 0)   map->renderorder = TILED_RENDERORDER_LEFT_UP;
        }
        else if (strcmp(name, "tiledversion") == 0) { /* TODO */ }
        else if (strcmp(name, "tileheight") == 0) map->tileheight = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "tilesets") == 0)
        {
            // iterate through array
            struct json_array_s* array = ((struct json_array_s*) elem->value->payload);
            size_t arr_len = array->length;
            // NOTE we could dynamically allocate memory for the tilesets array based on arr_len here
            for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
            {
                // NOTE tilesets inside a map only contain the gid & the filename of the tileset when the
                // tilemap is not exported w/ the export option "embed tileset". Currently we only support
                // embedded tilesets
                tiled_tileset_t* tileset = &map->tilesets[map->tileset_count++];
                for (json_object_element_s* obj = ((json_object_element_s*) elem->value)->next;
                     obj != NULL; obj = obj->next)
                {
                    const char* name = obj->name->string;

                    if (strcmp(name, "columns") == 0) tileset->columns = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "firstgid") == 0)
                        tileset->firstgid = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "image") == 0) copy_json_string(tileset->image, sizeof(tileset->image),
                                                                          json_value_as_string(obj->value)->string);
                    else if (strcmp(name, "imageheight") == 0)
                        tileset->imageheight = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "imagewidth") == 0)
                        tileset->imagewidth = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "margin") == 0)
                        tileset->margin = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "name") == 0) copy_json_string(tileset->name, sizeof(tileset->name),
                                                                         json_value_as_string(obj->value)->string);
                    else if (strcmp(name, "source") == 0) copy_json_string(tileset->source, sizeof(tileset->source),
                                                                           json_value_as_string(obj->value)->string);
                    else if (strcmp(name, "spacing") == 0)
                        tileset->spacing = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "tilecount") == 0)
                        tileset->tilecount = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "tileheight") == 0)
                        tileset->tileheight = atoi(json_value_as_number(obj->value)->number);
                    else if (strcmp(name, "tiles") == 0)
                    {
                        struct json_array_s* array = ((struct json_array_s*) obj->value->payload);
                        size_t arr_len = array->length;
                        // NOTE we could dynamically allocate memory for the tiles array based on arr_len here
                        for (json_array_element_s* elem = array->start; elem != nullptr; elem = elem->next)
                        {
                            tiled_tile_t* tile = &tileset->tiles[tileset->tile_count++];
                            for (json_object_element_s* obj = ((json_object_element_s*) elem->value)->next;
                                 obj != NULL; obj = obj->next)
                            {
                                const char* name = obj->name->string;
                                if (strcmp(name, "animation") == 0)
                                {
                                    // TODO iterate through the array of frames
                                }
                                else if (strcmp(name, "id") == 0)
                                {
                                    tile->id = atoi(json_value_as_number(obj->value)->number);
                                }
                                else if (strcmp(name, "image") == 0)
                                    copy_json_string(tile->image, sizeof(tile->image),
                                                     json_value_as_string(obj->value)->string);
                                else if (strcmp(name, "imageheight") == 0)
                                {
                                    tile->imageheight = atoi(json_value_as_number(obj->value)->number);
                                }
                                else if (strcmp(name, "imagewidth") == 0)
                                    tile->imagewidth = atoi(json_value_as_number(obj->value)->number);
                                else if (strcmp(name, "objectgroup") == 0)
                                {
                                    tiled_layer_t* obj_group = &tile->objectgroup;
                                    obj_group->type = TILED_LAYER_TYPE_OBJECTGROUP;
                                    for (json_object_element_s* elem = ((json_object_element_s*) obj->value)->next;
                                         elem != NULL; elem = elem->next)
                                    {
                                        const char* name = elem->name->string;
                                        // TODO
                                        if (strcmp(name, "draworder") == 0) ;
                                        else if (strcmp(name, "id") == 0) ;
                                        else if (strcmp(name, "name") == 0) ;
                                        else if (strcmp(name, "objects") == 0)
                                        {
                                            fill_objects_array(elem->value, obj_group);
                                        }
                                        else if (strcmp(name, "opacity") == 0) ;
                                        else if (strcmp(name, "type") == 0) ;
                                        else if (strcmp(name, "visible") == 0) ;
                                        else if (strcmp(name, "x") == 0) ;
                                        else if (strcmp(name, "y") == 0) ;
                                        else UNREACHABLE("unknown attribut '%s' for objectgroup layer\n", name);
                                    }
                                }
                                else if (strcmp(name, "type") == 0)
                                    copy_json_string(tile->type, sizeof(tile->type),
                                                     json_value_as_string(obj->value)->string);
                                else UNREACHABLE("unknown attribut '%s' for tile\n", name)
                            }
                        }
                    }
                    else if (strcmp(name, "tilewidth") == 0)
                        tileset->tilewidth = atoi(json_value_as_number(obj->value)->number);
                    else UNREACHABLE("unknown attribute '%s' for tileset\n", name);
                }
            }
        }
        else if (strcmp(name, "tilewidth") == 0) map->tilewidth = atoi(json_value_as_number(elem->value)->number);
        else if (strcmp(name, "type") == 0)
            copy_json_string(map->type, sizeof(map->type), "map"); // NOTE no other types as of 1.7.2
        else if (strcmp(name, "version") == 0) { /* TODO */ }
        else if (strcmp(name, "width") == 0) map->width = atoi(json_value_as_number(elem->value)->number);
        else UNREACHABLE("Unknown attribut '%s' for map", name);
    }

    free(json_dom); // NOTE we can only free this bc we copy out the strings
    } // TIMED_BLOCK

    //////////////////////////////////////////////////////////////////////////////////////////////

    // use map for level generation
    //u32 max_tiles                   = map->width * map->height;
    const tiled_layer_t* layers       = map->layers;
    const tiled_tileset_t* tilesets   = map->tilesets;
    const tiled_tileset_t* ts         = nullptr;

    for(u32 i = 0; i < map->layer_count; i++)
    {
        // for items & characters
        if (layers[i].type == TILED_LAYER_TYPE_OBJECTGROUP)
        {
            for (u32 j = 0; j < layers[i].obj_count; j++)
            {
                const tiled_object_t* o = &layers[i].objects[j];
                const char* type = o->type;
                // const char* name = o.name;
                Entity newEnt = {0};
                newEnt.active       = true;
                newEnt.freed        = false;
                newEnt.renderLayer  = i;

                const tiled_tile_t* t = nullptr;
                ts                    = nullptr;
                // determine tileset for this object
                // TODO is there a direct way?
                for (int k = 0; k < map->tileset_count; k++)
                {
                    // TODO write a utils function & use that here
                    u32 last_gid = map->tilesets[k].firstgid + map->tilesets[k].tilecount;
                    if (o->gid >= map->tilesets[k].firstgid && o->gid < last_gid)
                    {
                        ts = &map->tilesets[k];
                        break;
                    }

                }
                ASSERT(ts != nullptr);

                u32 tile_id = o->gid - ts->firstgid; // local tile ID
                i32 row_idx = tile_id % ts->columns;
                i32 col_idx = tile_id / ts->columns;
                u32 x_pos   = ts->margin + row_idx * (ts->tilewidth  + ts->spacing);
                u32 y_pos   = ts->margin + col_idx * (ts->tileheight + ts->spacing);

                // get collider
                rect_t collider = {0};
                const tiled_tile_t* special_tile = nullptr;
                // find special tile in tileset
                for (int k = 0; k < ts->tile_count; k++)
                {
                    if (ts->tiles[k].id == tile_id)
                    {
                        special_tile = &ts->tiles[k];
                        // special tile has collision data
                        if (special_tile->objectgroup.obj_count != 0)
                        {
                            collider = {(i32) special_tile->objectgroup.objects[0].x,
                                        (i32) special_tile->objectgroup.objects[0].y,
                                        (i32) special_tile->objectgroup.objects[0].width,
                                        (i32) special_tile->objectgroup.objects[0].height};
                        }
                    }
                }
                //collider = {0, 0, 16, 32}; // NOTE hardcoded aabb

                rect_t spritebox = { (i32) x_pos, (i32) y_pos, (i32) o->width, (i32) o->height };

                // TODO entity_create_character()
                if (strcmp(type, "Character") == 0)
                {
                    // TODO load in anims in here
                    // load in animations TODO this only needs to be loaded in
                    // once & not per character
                    //u32 anim_idx = 0;
                    //for (auto anim : animMap.getAnimatedTiles())
                    //{
                    //    std::vector<AnimationFrame> new_frames;
                    //    //u32 frame_count = 0;
                    //    for (auto frame : anim.second.animation.frames)
                    //    {
                    //        auto tileID = frame.tileID;
                    //        // TODO get proper tileset
                    //        auto pos    = animMap.getTilesets().at(0).getTile(tileID)->imagePosition;
                    //        auto size   = animMap.getTilesets().at(0).getTile(tileID)->imageSize;
                    //        //auto pos    = ts->getTile(tileID)->imagePosition;
                    //        //auto size   = ts->getTile(tileID)->imageSize;
                    //        rect_t bb = {(i32) pos.x,  (i32) pos.y, (i32) size.x, (i32) size.y};
                    //        new_frames.push_back({bb, (f32) frame.duration/100.f}); // TODO why cast?
                    //    }
                    //    newEnt.clips[newEnt.clip_count].frames = new_frames;
                    //    newEnt.clips[newEnt.clip_count].loop   = true;
                    //    newEnt.clip_count++;
                    //    newEnt.flags       |= ENT_FLAG_IS_ANIMATED;
                    //}

                    // TODO charID
                    newEnt.sprite.box   = spritebox;
                    newEnt.sprite.pivot = {0.5f, 0.5f};
                    newEnt.state        = ENT_STATE_MOVE;
                    newEnt.setPivPos( { (f32) o->x, (f32) o->y - 24, 0}); // TODO why -24
                    newEnt.sprite.tex   = resourcemgr_texture_load(ts->image, game_state); // TODO platform code (?)
                    newEnt.renderLayer  = 1;
                    newEnt.orient       = ENT_ORIENT_DOWN;
                    newEnt.collider     = collider;
                    newEnt.flags       |= ENT_FLAG_IS_COLLIDER;
                    //newEnt.flags       |= ENT_FLAG_PLAYER_CONTROLLED;
                    newEnt.flags       |= ENT_FLAG_CMD_CONTROLLED;
                    newEnt.flags       |= ENT_FLAG_IS_REWINDABLE;
                    Rewind::initializeFrames(newEnt);
                    command_init(newEnt);

                } else if (strcmp(type, "Item") == 0) {
                    newEnt.sprite.box   = spritebox;
                    newEnt.sprite.pivot = {0.5f, 0.75f};
                    newEnt.setPivPos( { (f32) o->x, (f32) o->y - 24, 0}); // TODO why -24
                    newEnt.sprite.tex   = resourcemgr_texture_load(ts->image, game_state);
                    newEnt.collider     = collider;
                    newEnt.flags       |= ENT_FLAG_IS_COLLIDER;
                    newEnt.flags       |= ENT_FLAG_IS_ITEM;
                    newEnt.flags       |= ENT_FLAG_IS_REWINDABLE;
                    Rewind::initializeFrames(newEnt);
                }
                EntityMgr::copyEntity(newEnt); // copy new entity into array TODO slow
            } // object loop
        } // objectlayer

        // for static tiles w/ and w/o colliders
        if (layers[i].type == TILED_LAYER_TYPE_TILELAYER)
        {
            for (u32 tile_idx = 0; tile_idx < layers[i].tile_count; tile_idx++)
            {
                u32 tile_gid = layers[i].data[tile_idx];
                ts = nullptr;
                // determine tileset for this object
                // TODO is there a direct way?
                for (int k = 0; k < map->tileset_count; k++)
                {
                    auto ts_tilecount = map->tilesets[k].tilecount;
                    auto last_gid = map->tilesets[k].firstgid + ts_tilecount;

                    // TODO if (tileset_has_tile(tile_idx))
                    if (tile_gid >= map->tilesets[k].firstgid &&
                        tile_gid <= (map->tilesets[k].firstgid + last_gid))
                    {
                        ts = &map->tilesets[k];
                        break;
                    }
                }
                if (!ts) { continue; }
                ASSERT(ts != nullptr);

                u32 tile_id = tile_gid - ts->firstgid; // local tileID
                i32 row_idx = tile_id % ts->columns;
                i32 col_idx = tile_id / ts->columns;
                u32 x_pos   = ts->margin + row_idx * (ts->tilewidth  + ts->spacing);
                u32 y_pos   = ts->margin + col_idx * (ts->tileheight + ts->spacing);

                // to create the spritebox
                rect_t bb = { (i32) x_pos, (i32) y_pos, (i32) ts->tilewidth, (i32) ts->tileheight };

                // CONSTRUCT TILE
                Tile newTile = {0};

                const tiled_tile_t* special_tile = nullptr;
                // find special tile in tileset
                for (int k = 0; k < ts->tile_count; k++)
                {
                    if (ts->tiles[k].id == tile_id)
                    {
                        special_tile = &ts->tiles[k];
                        // special tile has collision data
                        if (special_tile->objectgroup.obj_count != 0)
                        {
                            newTile.collider = {(i32) special_tile->objectgroup.objects[0].x,
                                                (i32) special_tile->objectgroup.objects[0].y,
                                                (i32) special_tile->objectgroup.objects[0].width,
                                                (i32) special_tile->objectgroup.objects[0].height};
                            newTile.collidable = true;
                        }
                    }
                }

                // TODO the foremost layer (Tile Layer 2 w/ the trees and the
                // grid as of rn) is not properly lined up w/ the rest. This
                // only seems to be the case for the horizontal purple grid
                // elements. Compare preview in Tiled w/ the game to see this.

                u32 x = tile_idx % layers[i].width;
                u32 y = tile_idx / layers[i].height;

                newTile.renderLayer  = i;
                newTile.sprite.box   = bb;
                newTile.sprite.pivot = {0.5f, 0.5f};
                newTile.sprite.tex   = resourcemgr_texture_load(ts->image, game_state);
                newTile.setPivPos({x * 16.f, y * 16.f, 0}); // TODO hardcoded

                EntityMgr::createTile(newTile); // copy new tile into array TODO slow
            } // tile loop
        } // tilelayer
    } // layer loop

    //free(map); // NOTE we need to make sure not to keep any pointers to the
                 // strings inside map, which is the case rn for the hash table of
                 // the resourcemgr

    return true;
}
