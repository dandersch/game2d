#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "utils.h"

typedef u32 ui_id;

struct ui_t
{
    v2i mouse_pos;
    b32 mouse_pressed;

    texture_t* btn_texture;

    ui_id curr_focus;
    ui_id curr_active;
    ui_id last_focus;
};

// TODO maybe add & use a float rect_t
inline b32 ui_button(ui_t* ctx, rect_t btn_rect, ui_id id)
{
    b32 pressed = false;
    if (ctx->curr_focus == id)
    {
        if (ctx->mouse_pressed)
        {
            if (ctx->curr_focus == id)
            {
                pressed = true;
            }
        }
    }

    if (utils_point_in_rect(ctx->mouse_pos, btn_rect))
        ctx->curr_focus = id;
    else
        ctx->curr_focus = 0;

    /* render */
    platform.renderer.push_texture({ctx->btn_texture, {0}, btn_rect});

    return pressed;
}

#endif // INTERFACE_H_
