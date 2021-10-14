#include "resourcemgr.h"
#include "platform.h"
#include "memory.h"

// TODO load in "missing" placeholder assets

#define HASH_TABLE_SIZE 256 // NOTE hashtable supports no reallocation
struct ht_entry_t
{
    void* value;
    const char* key;
};
ht_entry_t hash_table_backend[HASH_TABLE_SIZE] = {0}; // NOTE should be in game_state?

// TODO better hash function. see e.g. https://cp-algorithms.com/string/string-hashing.html
internal u32 hash_function(const char* key)
{
    u32 ht_idx = 0;
    for (int i = 0; i < strlen(key); i++)
    {
        ht_idx += key[i];
    }

    // mask off high bits so that the index is under the hash table size
    ht_idx &= (sizeof(hash_table_backend)/sizeof((hash_table_backend)[0])) - 1;

    ASSERT(ht_idx < HASH_TABLE_SIZE);

    return ht_idx;
}

internal b32 hash_table_add_entry(const char* key, void* value)
{
    u32 hash = hash_function(key);
    b32 wrapped_around = false;

    // collision handling w/ internal chaining
    for (u32 idx = hash; idx < HASH_TABLE_SIZE; idx++)
    {
        if (hash_table_backend[idx].key == NULL) // no collision
        {
            hash_table_backend[idx] = {value, key};
            return true;
        }

        // array is completely filled
        if (idx == hash && wrapped_around) return false;

        // handle wraparound
        if (idx == (HASH_TABLE_SIZE - 1))
        {
            wrapped_around = true;
            idx = 0;
        }
    }

    return false;
}

internal void* hash_table_get_value(const char* key)
{
    u32 hash = hash_function(key);
    b32 wrapped_around = false;
    void* result = nullptr;

    for (u32 idx = hash; idx < HASH_TABLE_SIZE; idx++)
    {
        if (hash_table_backend[idx].key == NULL) // we early out if there is no key
        {
            result = nullptr;
            return result;
        }

        if (strcmp(hash_table_backend[idx].key, key) == 0) // key found
        {
            result = hash_table_backend[idx].value;
            return result;
        }

        if (idx == hash && wrapped_around) // array completely searched
        {
            result = nullptr;
            return result;
        }

        if (idx == (HASH_TABLE_SIZE - 1)) // handle wraparound
        {
            wrapped_around = true;
            idx = 0;
        }
    }

    result = nullptr;
    return result;
}

// NOTE we use this to concatenate every filename w/ the resource folder "res/"
// this means filepaths can't be longer than 256 characters
#define MAX_CHARS_FOR_FILEPATH 256
#define RESOURCE_FOLDER "res/"
texture_t resourcemgr_texture_load(const char* filename, game_state_t* state)
{
    char filepath[MAX_CHARS_FOR_FILEPATH] = RESOURCE_FOLDER;
    ASSERT(strlen(filename) < (MAX_CHARS_FOR_FILEPATH - strlen(RESOURCE_FOLDER)));
    strcat(filepath, filename);

    texture_t tex = hash_table_get_value(filename);
    if (tex) return tex;
    else // not found
    {
        tex = platform.renderer.load_texture(state->window, filepath);
        hash_table_add_entry(filename, tex);
        if (!tex) return hash_table_get_value("missing"); // TODO handle if this fails
    }

    return tex;
}

font_t* resourcemgr_font_load(const char* filename, game_state_t* state, i32 ptsize)
{
    char filepath[MAX_CHARS_FOR_FILEPATH] = RESOURCE_FOLDER;
    ASSERT(strlen(filename) < (MAX_CHARS_FOR_FILEPATH - strlen(RESOURCE_FOLDER)));
    strcat(filepath, filename);

    font_t* tex = hash_table_get_value(filename);
    if (tex) return tex;
    else // not found
    {
        tex = platform.font_load(filepath, ptsize);
        hash_table_add_entry(filename, tex);
        if (!tex) return hash_table_get_value("missing"); // TODO handle if this fails
    }

    return tex;
}

// TODO
b32 resourcemgr_free(const char* filename, game_state_t* state)
{
    //if (state->pool_textures.find(filename) != state->pool_textures.end()) // found
    //{
    //    // TODO free_resource() for all Resources
    //    return true;
    //}
    return false; // not found
}
