#include "platform_renderer.h"
#include "utils.h"
#include "memory.h"

/* This is the implementation of the renderer api with the SDL renderer, which
 * can use OpenGL, OpenGLES, D3D, Metal, Software rendering and possibly more
 * APIs as its backend. It doesn't have as many features as e.g. raw OpenGL, so
 * it could/should be used as a fallback. */

struct texture_t
{
    SDL_Texture* id;
};

global_var renderer_cmd_buf_t* cmds;

void renderer_init(platform_window_t* window, mem_arena_t* platform_mem_arena)
{
    cmds = (renderer_cmd_buf_t*) mem_arena_alloc(platform_mem_arena, sizeof(cmds));
    cmds->buf_offset  = cmds->buf;
    cmds->entry_count = 0;

    // TODO needs to be in renderer_sdl.cpp
    window->renderer = (renderer_t*) SDL_CreateRenderer(window->handle, -1, SDL_RENDERER_ACCELERATED
                                                                          /*| SDL_RENDERER_PRESENTVSYNC */);
    SDL_ERROR(window->renderer);
    //SDL_RenderSetLogicalSize(rw->renderer, 640, 480);
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    //printf("%s\n", SDL_GetHint("SDL_HINT_RENDER_SCALE_QUALITY")); SDL_RenderSetScale((SDL_Renderer*) window->renderer, 1.f, 1.f);
}

void renderer_destroy(renderer_t* renderer)
{
    SDL_DestroyRenderer((SDL_Renderer*) renderer);
}

// 0 on success, -1 if texture is not valid
i32 renderer_texture_query(texture_t* tex, u32* format, i32* access, i32* w, i32* h)
{
    return SDL_QueryTexture(tex->id, format, access, w, h);
}

texture_t* renderer_create_texture_from_surface(platform_window_t* window, surface_t* surface)
{
    SDL_Texture* sdl_tex = SDL_CreateTextureFromSurface((SDL_Renderer*) window->renderer, (SDL_Surface*) surface);
    SDL_ERROR(sdl_tex);

    texture_t* tex = (texture_t*) malloc(sizeof(texture_t));
    tex->id = sdl_tex;

    return tex;
}

texture_t* renderer_load_texture(platform_window_t* window, const char* filename)
{
    SDL_Texture* sdl_tex = IMG_LoadTexture((SDL_Renderer*) window->renderer, filename);
    SDL_ERROR(sdl_tex);

    texture_t* tex = (texture_t*) malloc(sizeof(texture_t));
    tex->id = sdl_tex;

    return tex;
}

internal_fn
SDL_BlendMode sdl_blendmode_lut(texture_blend_mode_e ours)
{
    switch(ours)
    {
        case TEXTURE_BLEND_MODE_NONE:  return SDL_BLENDMODE_NONE;
        case TEXTURE_BLEND_MODE_BLEND: return SDL_BLENDMODE_BLEND;
        case TEXTURE_BLEND_MODE_ADD:   return SDL_BLENDMODE_ADD;
        case TEXTURE_BLEND_MODE_MOD:   return SDL_BLENDMODE_MOD;
        case TEXTURE_BLEND_MODE_MUL:   return SDL_BLENDMODE_MUL;
        default: return SDL_BLENDMODE_INVALID;
    }
}

void renderer_cmd_buf_process(platform_window_t* window)
{
    SDL_Renderer* renderer = (SDL_Renderer*) window->renderer;

    u8* curr_entry = cmds->buf;
    for (s32 entry_nr = 0; entry_nr < cmds->entry_count; ++entry_nr)
    {
        render_entry_header_t* entry_header = (render_entry_header_t*) curr_entry;
        switch (entry_header->type)
        {
            case RENDER_ENTRY_TYPE_TEXTURE:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_texture_t* draw_tex = (render_entry_texture_t*) curr_entry;

                // NOTE we need to do this here because SDL_RenderCopy expects
                // pointers & NULL has special meaning (and a nullptr doesn't do
                // the same things as an empty rectangle)
                SDL_Rect* src = (SDL_Rect*) &draw_tex->src;
                SDL_Rect* dst = (SDL_Rect*) &draw_tex->dst;
                if (utils_rect_empty(draw_tex->src)) src = NULL;
                if (utils_rect_empty(draw_tex->dst)) dst = NULL;

                SDL_RenderCopy(renderer, draw_tex->tex->id, src, dst);
                curr_entry += sizeof(render_entry_texture_t);
            } break;

            case RENDER_ENTRY_TYPE_RECT:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;

                SDL_SetRenderDrawColor(renderer, rect->color.r, rect->color.g, rect->color.b, rect->color.a);
                SDL_RenderDrawRect(renderer, (SDL_Rect*) &rect->rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

                curr_entry += sizeof(render_entry_rect_t);
            } break;

            case RENDER_ENTRY_TYPE_TEXTURE_MOD:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_texture_mod_t* mod = (render_entry_texture_mod_t*) curr_entry;

                if (!(mod->blend == TEXTURE_BLEND_MODE_NO_CHANGE))
                    SDL_SetTextureBlendMode(mod->tex->id, sdl_blendmode_lut(mod->blend));
                if (!(mod->scale == TEXTURE_SCALE_MODE_NO_CHANGE))
                    SDL_SetTextureScaleMode(mod->tex->id, (SDL_ScaleMode) mod->scale);

                // NOTE we lose the ability to change the alpha w/o touching the color & vice versa
                SDL_SetTextureColorMod(mod->tex->id, mod->rgba.r, mod->rgba.g, mod->rgba.b);
                SDL_SetTextureAlphaMod(mod->tex->id, mod->rgba.a);

                curr_entry += sizeof(render_entry_texture_mod_t);
            } break;

            case RENDER_ENTRY_TYPE_CLEAR:
            {
                curr_entry += sizeof(render_entry_header_t);
                SDL_RenderClear(renderer);
                curr_entry += sizeof(render_entry_clear_t);
            } break;

            case RENDER_ENTRY_TYPE_PRESENT:
            {
                curr_entry += sizeof(render_entry_header_t);
                SDL_RenderPresent(renderer);
                curr_entry += sizeof(render_entry_present_t);
            } break;

            default:
            {
                UNREACHABLE("SDL render command for entry '%u' not implemented", entry_header->type);
            }
        }
    }

    // reset cmd buffer
    cmds->entry_count = 0;
    cmds->buf_offset  = cmds->buf;
}
