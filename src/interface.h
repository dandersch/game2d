#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "utils.h"

typedef u32 ui_id;

// TODO embed a bitmap font

enum ui_color_e
{
  UI_COLOR_TEXT,
  //UI_COLOR_BORDER,
  //UI_COLOR_WINDOWBG,
  //UI_COLOR_TITLEBG,
  //UI_COLOR_TITLETEXT,
  //UI_COLOR_PANELBG,
  UI_COLOR_BUTTON,
  UI_COLOR_BUTTONHOVER,
  UI_COLOR_BUTTONFOCUS,
  //UI_COLOR_BASE,
  //UI_COLOR_BASEHOVER,
  //UI_COLOR_BASEFOCUS,
  //UI_COLOR_SCROLLBASE,
  //UI_COLOR_SCROLLTHUMB,
  UI_COLOR_COUNT
};
struct ui_style_t
{
    //font; size; padding; spacing; indent;
    //title_height; scrollbar_size; thumb_size;
    colorf_t colors[UI_COLOR_COUNT];
};


#define UI_ELEMENTS_MAX 100
struct ui_t
{
    v2i mouse_pos;
    b32 mouse_pressed;

    texture_t* btn_texture;

    ui_style_t style;

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
    colorf_t btn_color =  ctx->style.colors[UI_COLOR_BUTTON];

    if (utils_point_in_rect(ctx->mouse_pos, btn_rect))
        ctx->curr_focus = id;

    b32 pressed = false;
    if (ctx->curr_focus == id)
    {
        btn_color = ctx->style.colors[UI_COLOR_BUTTONHOVER];
        if (ctx->mouse_pressed)
        {
            if (ctx->curr_focus == id)
            {
                pressed = true;
            }
        }
    }

    /* push elements to render */
    ctx->render_buf[ctx->render_elems_count++] = {ctx->btn_texture, {0}, btn_rect, btn_color};
    //if (sprite) ctx->render_buf[ctx->render_elems_count++] = {sprite->tex, sprite->box, btn_rect};
    if (sprite)
    {
        rect_t dst = {btn_rect.left + 37, btn_rect.top + 25, btn_rect.w - 75, btn_rect.h - 50};
        ctx->render_buf[ctx->render_elems_count++] = {sprite->tex, sprite->box, dst};
    }

    return pressed;
}


/*
bool doButton(UI, ID, Text, ...)
{
    if (active)
    {
        if (MouseWentUp)
        {
            if (hot) result = true;
            SetNotActive();
        }
        else if (hot)
        {
            if (MouseWentDown) SetActive(),
        }
    }

    if (Inside()) SetHot();
}
*/


inline void ui_render(ui_t* ctx, platform_api_t* platform)
{
    for (u32 i = 0; i < ctx->render_elems_count; i++)
        platform->renderer.push_texture(ctx->render_buf[i]);
}

#endif // INTERFACE_H_
