#pragma once
#include "pch.h"
#include "renderwindow.h"

// TODO use unique_ptr
// TODO if we want to use templates here we probably need to wrap SDL_Textures,
// fonts etc.
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
            pool[file] = IMG_LoadTexture(rw->renderer, file.c_str());
            SDL_ERROR(pool[file]);
            if (!pool[file]) return pool["missing"];
        }
        return pool[file];
    }

private:
    static std::unordered_map<std::string, Resource> pool;
};

template <typename Resource>
std::unordered_map<std::string, Resource> ResourceManager<Resource>::pool{};
