#include "platform_renderer.h"
#include "utils.h"

/* This is the implementation of the renderer api with the SDL renderer, which
 * can use OpenGL, OpenGLES, D3D, Metal, Software rendering and possibly more
 * APIs as its backend. It doesn't have as many features as e.g. raw OpenGL, so
 * it could/should be used as a fallback. */

void renderer_init(platform_window_t* window)
{
    cmds.buf_offset  = cmds.buf;
    cmds.entry_count = 0;

    // TODO needs to be in renderer_sdl.cpp
    window->renderer = (renderer_t*) SDL_CreateRenderer(window->handle, -1, SDL_RENDERER_ACCELERATED
                                                                          /*| SDL_RENDERER_PRESENTVSYNC */);
    SDL_ERROR(window->renderer);
    //SDL_RenderSetLogicalSize(rw->renderer, 640, 480);
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    //printf("%s\n", SDL_GetHint("SDL_HINT_RENDER_SCALE_QUALITY"));
    SDL_RenderSetScale((SDL_Renderer*) window->renderer, 1.f, 1.f);
}

void renderer_destroy(renderer_t* renderer)
{
    SDL_DestroyRenderer((SDL_Renderer*) renderer);
}

void renderer_cmd_buf_process(platform_window_t* window)
{
    SDL_Renderer* renderer = (SDL_Renderer*) window->renderer;

    u8* curr_entry = cmds.buf;
    for (s32 entry_nr = 0; entry_nr < cmds.entry_count; ++entry_nr)
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

                SDL_RenderCopy(renderer, (SDL_Texture*) draw_tex->tex, src, dst);
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

            case RENDER_ENTRY_TYPE_CLEAR:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;
                SDL_RenderClear(renderer);
                curr_entry += sizeof(render_entry_clear_t);
            } break;
            case RENDER_ENTRY_TYPE_PRESENT:
            {
                curr_entry += sizeof(render_entry_header_t);
                render_entry_rect_t* rect = (render_entry_rect_t*) curr_entry;
                SDL_RenderPresent(renderer);
                curr_entry += sizeof(render_entry_present_t);
            } break;
            default:
            {
                UNREACHABLE("Render command for entry '%u' not implemented", entry_header->type);
            }
        }
    }

    // reset cmd buffer
    cmds.entry_count = 0;
    cmds.buf_offset  = cmds.buf;
}
