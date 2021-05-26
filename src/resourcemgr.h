#pragma once
#include "pch.h"
#include "renderwindow.h"

// TODO use unique_ptr/shared_ptr
// TODO if we want to use templates here we probably need to wrap SDL_Textures,
// fonts etc.
// TODO ptsize hardcoded
void loadFromFile(TTF_Font** fnt, const char* file);
void loadFromFile(SDL_Texture** tex, const char* file);

template<typename Resource>
class ResourceManager
{
public:
    static Resource get(const std::string& file)
    {
        if (pool.find(file) != pool.end()) // found
        {
            return pool[file];
        }
        else // not found
        {
            loadFromFile(&pool[file], file.c_str());
            SDL_ERROR(pool[file]);
            if (!pool[file]) return pool["missing"];
        }
        return pool[file];
    }

    static bool free(const std::string& file)
    {
        if (pool.find(file) != pool.end()) // found
        {
            // TODO freeResource() for all Resources
            return true;
        }
        return false; // not found
    }

private:
    static std::unordered_map<std::string, Resource> pool;
};

template <typename Resource>
std::unordered_map<std::string, Resource> ResourceManager<Resource>::pool{};
