#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "utils.h"

typedef u32 ui_id;

// TODO embed a bitmap font

#define UI_ELEMENTS_MAX 100
struct ui_t
{
    v2i mouse_pos;
    b32 mouse_pressed;

    texture_t* btn_texture;
    colorf_t   btn_color;
    rect_t btn_spritebox;

    render_entry_texture_t render_buf[UI_ELEMENTS_MAX];
    u32 render_elems_count;
    b32 updated_this_frame;

    ui_id curr_focus;
    ui_id curr_active;
    ui_id last_focus;
};


inline void ui_init()
{

}


inline void ui_begin(ui_t* ctx)
{
    ctx->render_elems_count = 0;
}


// TODO maybe add & use a float rect_t
inline b32 ui_button(ui_t* ctx, rect_t btn_rect, sprite_t* sprite, ui_id id)
{
    if (utils_point_in_rect(ctx->mouse_pos, btn_rect))
        ctx->curr_focus = id;

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

    /* push elements to render */
    ctx->render_buf[ctx->render_elems_count++] = {ctx->btn_texture, {0}, btn_rect, ctx->btn_color};
    if (sprite) ctx->render_buf[ctx->render_elems_count++] = {sprite->tex, sprite->box, btn_rect};

    return pressed;
}


inline void ui_render(ui_t* ctx, platform_api_t* platform)
{
    for (u32 i = 0; i < ctx->render_elems_count; i++)
        platform->renderer.push_texture(ctx->render_buf[i]);
}

#endif // INTERFACE_H_
