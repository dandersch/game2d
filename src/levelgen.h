#pragma once

// see https://doc.mapeditor.org/en/stable/reference/json-map-format/

struct tiled_object_t
{
    // b32 ellipse
    // u32 gid; // Global tile ID, only if object represents a tile
    f64 height; // TODO why double
    u32 id; // incremental ID, unique across all objects
    // char* name;
    // b32 point
    // polygon[]
    // polyline[]
    // properties[]
    // rotation
    // template
    // text
    // char* type // custom string
    b32 visible;
    f64 width; // TODO why double
    u32 x; // x coordinate in pixels
    u32 y; // y coordinate in pixels
};

struct tiled_frame_t
{
    u32 duration; // Frame duration in milliseconds
    u32 tileid;   // local tile ID representing this frame
};

enum tiled_layer_type_e
{
    TILED_LAYER_TYPE_TILELAYER,
    TILED_LAYER_TYPE_OBJECTGROUP,
    TILED_LAYER_TYPE_IMAGELAYER,
    TILED_LAYER_TYPE_GROUP,
    TILED_LAYER_TYPE_COUNT
};
#define TILED_MAX_GIDS_PER_LAYER 100000  // TODO find a better max
#define TILED_MAX_OBJECTS_PER_LAYER 1000 // TODO find a better max
struct tiled_layer_t
{
    // A TILELAYER IN TILED IS MADE UP OUT OF:
    // tiled_chunk_t chunks[]; // optional. tilelayer only
    // char* compression; // zlib, gzip, zst, empty (default). tilelayer only.
    u32 data[TILED_MAX_GIDS_PER_LAYER]; // GIDs that map to a sprite from a tileset,
                                        // tilelayer only
    b32 draworder; // 0: topdown, 1: index. Objectgroup only
    // char* encoding // csv or base64. tilelayer only
    u32 height; // Row count. Same as map height for fixed-size maps.
    u32 id;     // Incremental ID - unique across all layers
    // char* image;              // imagelayer only
    // tiled_layer_t layers[];   // group only
    const char* name;
    tiled_object_t objects[TILED_MAX_OBJECTS_PER_LAYER]; // objectgroup only
    // f64 offsetx; // Horizontal layer offset in pixels (default: 0)
    // f64 offsety; // Vertical layer offset in pixels (default: 0)
    f64 opacity; // from 0 to 1
    // f64 parallaxx // Horizontal parallax factor for this layer (default: 1).
    // f64 parallaxy // Vertical parallax factor for this layer (default: 1).
    // tiled_property_t properties[];
    // s32 startx; // for infinite maps
    // s32 starty;  // for infinite maps
    // char* tintcolor // hex-formatted color. optional.
    // char* transparentcolor // hex-formatted color. imagelayer only.
    tiled_layer_type_e type;
    b32 visible;
    u32 width; // Column count. Same as map height for fixed-size maps.
    u32 x; // Horizontal layer offset in tiles. Always 0.
    u32 y; // Vertical layer offset in tiles. Always 0.
};

// A tileset that associates information with each tile, like its image path or
// terrain type, may include a tiles array property. Each tile has an id
// property, which specifies the local ID within the tileset.
//
// For the terrain information, each value is a length-4 array where each
// element is the index of a terrain on one corner of the tile. The order of
// indices is: top-left, top-right, bottom-left, bottom-right.
#define TILED_MAX_FRAMES_FOR_TILE_ANIMATION 10 // TODO find better max
struct tiled_tile_t
{
    tiled_frame_t animation[TILED_MAX_FRAMES_FOR_TILE_ANIMATION];
    u32 id;                    // Local ID of the tile
    char* image;               // Image representing this tile (optional)
    u32 imageheight;           // Height of the tile image in pixels
    u32 imagewidth;            // Width of the tile image in pixels
    //tiled_layer_t objectgroup; // Layer with type objectgroup, when collision
                                 // shapes are specified (optional)
                                 // TODO causes recursion
    // f64 probability // percentage chance this tile is chosen when competing with others in the editor (optional)
    // properties // Array of Properties
    // terrain[]  // Index of terrain for each corner of tile (optional)
    char* type; // The type of the tile (optional)

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
};

#define TILED_MAX_TILES_IN_TILESET 1000 // TODO find better max
struct tiled_tileset_t
{
    //char* backgroundcolor; // Hex-formatted color (#RRGGBB or #AARRGGBB) (optional)
    u32   columns;         // The number of tile columns in the tileset
    u32   firstgid;        // GID corresponding to the first tile in the set
    // tiled_grid_t grid   // Grid (optional)
    char* image;           // Image used for tiles in this set
    u32 imageheight;       // Height of source image in pixels
    u32 imagewidth;        // Width of source image in pixels
    u32 margin;            // Buffer between image edge and first tile (pixels)
    char* name;            // Name given to this tileset
    //objectalignment        // Alignment to use for tile objects (unspecified (default), topleft, top, topright, left, center, right, bottomleft, bottom or bottomright) (since 1.4)
    //properties             // Array of Properties
    //char* source;          // The external file that contains this tilesets data
    u32 spacing;           // Spacing between adjacent tiles in image (pixels)
    // terrains[];           // Array of Terrains (optional)
    u32 tilecount;         // The number of tiles in this tileset
    // char* tiledversion;   // The Tiled version used to save the file
    u32 tileheight;        // Maximum height of tiles in this set
    // tiled_tiledoffset_t tileoffset;           // Tile Offset (optional)
    tiled_tile_t tiles[TILED_MAX_TILES_IN_TILESET];  // (optional)
    u32 tilewidth;         // Maximum width of tiles in this set
    // tiled_transformation_t transformations // Allowed transformations (optional)
    // char* transparentcolor // Hex-formatted color (#RRGGBB) (optional)
    // char* type            // "tileset"
    // char* version         // The JSON format version
    // wangsets[]            // array of wangsets
};

struct tiled_chunk_t
{
    // unused
};

enum tiled_orientation_e
{
    TILED_ORIENTATION_ORTHOGONAL,
    TILED_ORIENTATION_ISOMETRIC,
    TILED_ORIENTATION_STAGGERED,
    TILED_ORIENTATION_HEXAGONAL,
    TILED_ORIENTATION_COUNT
};
enum tiled_renderorder_e
{
    TILED_RENDERORDER_RIGHT_DOWN,
    TILED_RENDERORDER_RIGHT_UP,
    TILED_RENDERORDER_LEFT_DOWN,
    TILED_RENDERORDER_LEFT_UP,
    TILED_RENDERORDER_COUNT
};

#define TILED_MAX_LAYERS   5
#define TILED_MAX_TILESETS 5
struct tiled_map_t
{
    // A MAP IN TILED IS MADE UP OUT OF:
    //
    // char* backgroundcolor // hex-formatted color;
    // s32 compressionlevel
    u32 height;
    // i32 hexsidelength;
    // b32 infinite
    tiled_layer_t layers[TILED_MAX_LAYERS];
    // u32 nextlayerid  // TODO what is this
    // u32 nextobjectid // TODO what is this
    tiled_orientation_e orientation;
    tiled_renderorder_e renderorder;
    // char* staggeraxis  // x or y
    // char* staggerindex // odd or even
    // char* tiledversion // e.g. "1.7.2"
    u32 tileheight;
    tiled_tileset_t tilesets[TILED_MAX_TILESETS]; // TODO actually just contains
                                                  // "firstgid" & "source" (path)
    u32 tilewidth;
    // char* type; // "map"
    // char* version // JSON format version
    u32 width;

    u32 layer_count = 0; // ours
};

struct tiled_text_t
{
    // unused
};
struct tiled_grid_t
{
    // unused
};
struct tiled_offset_t
{
    // unused
};
struct tiled_transformation_t
{
    // unused
};
struct tiled_object_template_t
{
    // unused
};
struct tiled_property_t
{
    // unused
};
struct tiled_point_t
{
    // unused
};
