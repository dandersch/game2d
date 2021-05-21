#pragma once
#include "pch.h"
#include "renderwindow.h"

// TODO make more robust w/o ext
// TODO use unique_ptr
// TODO if we want to use templates here we probably need to wrap SDL_Textures,
// fonts etc.
// TODO turn into singleton
template<typename Resource>
class ResourceManager
{
public:
    ResourceManager(const std::string& ext)
    {
        std::string path = "res/missing" + ext;
        pool["missing"] = IMG_LoadTexture(rw->renderer, path.c_str());
        SDL_ERROR(pool["missing"]);
    }

    Resource get(const std::string& file)
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
    //std::unordered_map<std::string, std::unique_ptr<Resource>> pool;
    std::unordered_map<std::string, Resource> pool;
};
