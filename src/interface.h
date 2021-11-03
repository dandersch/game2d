#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "utils.h"

typedef i32 ui_id;

// TODO embed a bitmap font
//#include "bitmap_font.h"

#define UI_ELEMENTS_MAX 100
enum ui_color_e
{
    UI_COLOR_TEXT,
    //UI_COLOR_BORDER,
    UI_COLOR_WINDOW,
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


// TODO possible improvements
// [ ] ability to center text
// [ ] automatic linebreaks
// [ ] fit font size to button size
struct ui_font_t
{
    const u32 FONT_WIDTH       = 128;
    const u32 FONT_HEIGHT      =  64;
    const u32 FONT_COLS        =  18;
    const u32 FONT_ROWS        =   7;
    const i32 FONT_CHAR_WIDTH  = (FONT_WIDTH / FONT_COLS);
    const i32 FONT_CHAR_HEIGHT = (FONT_HEIGHT / FONT_ROWS);
    const u32 FONT_SOLID_CHAR  = 127;
    texture_t* bitmap;
};

enum
{
    WINDOW_STYLE_HORIZONTAL,
    WINDOW_STYLE_VERTICAL,
    WINDOW_STYLE_COUNT
};
struct ui_window_t
{
    // top
    // left
    // width
    // height
    b32 active;
    rect_t rect;
    ui_id  id;
    u32 style = WINDOW_STYLE_VERTICAL;
    i32 zindex  = -1; // smallest zindex gets drawn last
    u32 padding = 15;
};

struct ui_t
{
    /* input */
    v2i mouse_pos;
    b32 mouse_pressed;

    ui_style_t style;
    rect_t btn_spritebox;
    ui_font_t font;

    ui_id curr_focus;
    ui_id curr_active;
    ui_id last_focus;
    ui_window_t curr_window;

    /* rendering */
    render_entry_texture_t render_buf[UI_ELEMENTS_MAX];
    u32 render_elems_count;

    /* unused */
    ui_id id_stack[UI_ELEMENTS_MAX]; // TODO per frame to check for id collisions
};


inline void ui_init(ui_t* ctx)
{
    ctx->style.colors[UI_COLOR_BUTTON]      = {0.6f, 0.2f, 0.2f, 1.0f};
    ctx->style.colors[UI_COLOR_BUTTONHOVER] = {0.8f, 0.2f, 0.2f, 1.0f};
    ctx->style.colors[UI_COLOR_WINDOW]      = {0.1,0.1,1.0,0.9};
    // TODO load in font texture
}


inline void ui_begin(ui_t* ctx)
{
    /* TODO assert */
    ctx->render_elems_count = 0;
    ctx->curr_window.active = 0;
    ctx->curr_focus         = __COUNTER__; // zero, TODO better id system
}


inline void ui_end(ui_t* ctx)
{
    //ASSERT(ctx->curr_window.active == false); // forgot to call ui_window_end?
}


inline void ui_window_begin(ui_t* ctx, i32 left, i32 top, ui_id id, u32 style = WINDOW_STYLE_VERTICAL)
{
    // TODO making the window 0 by 0 big makes it fill the entire screen right now
    ctx->curr_window = {1, {left, top, 1, 1}, id, style};
}


inline void ui_window_end(ui_t* ctx)
{
    switch (ctx->curr_window.style) // add last bit of padding
    {
        case WINDOW_STYLE_HORIZONTAL: { ctx->curr_window.rect.h += ctx->curr_window.padding; } break;
        case WINDOW_STYLE_VERTICAL:   { ctx->curr_window.rect.w += ctx->curr_window.padding; } break;
    }

    rect_t   dst   = ctx->curr_window.rect;
    i32      z_idx = ctx->curr_window.zindex;
    colorf_t color = ctx->style.colors[UI_COLOR_WINDOW];
    ctx->render_buf[ctx->render_elems_count++] = {nullptr, {0}, dst, z_idx, color};

    //ctx->curr_window.active = false;
}


inline rect_t ui_font_get_box_for_char(ui_font_t font, char c)
{
    i32 index = c - ' ';
    i32 row_idx = index / font.FONT_COLS ;
    i32 col_idx = index % font.FONT_COLS ;
    i32 x_pos = col_idx * font.FONT_CHAR_WIDTH;
    i32 y_pos = row_idx * font.FONT_CHAR_HEIGHT;
    return {x_pos, y_pos, font.FONT_CHAR_WIDTH, font.FONT_CHAR_HEIGHT};
}


// TODO maybe add & use a float rect_t
// TODO add an optional animator that updates the sprite when hovering over the button
inline b32 ui_button(ui_t* ctx, i32 width, i32 height, sprite_t* sprite, ui_id id, const char* text = nullptr)
{
    ASSERT(ctx->curr_window.active); // window now required

    colorf_t btn_color = ctx->style.colors[UI_COLOR_BUTTON];
    rect_t   btn_rect  = {0,0, width, height};

    // bump window size by padding + button size depending on window style
    // TODO refactor
    if (ctx->curr_window.style == WINDOW_STYLE_HORIZONTAL)
    {
        ui_window_t* window = &ctx->curr_window;
        window->rect.h += window->padding;

        if (window->rect.w <= 1) window->rect.w += (width + 2*window->padding);
        if (window->rect.w <= (width + 2*window->padding))
            window->rect.w += ((width + 2*window->padding) - window->rect.w);

        btn_rect.top  = window->rect.top  + window->rect.h;
        btn_rect.left = window->rect.left + window->padding;

        window->rect.h += height;
    }
    if (ctx->curr_window.style == WINDOW_STYLE_VERTICAL)
    {
        ui_window_t* window = &ctx->curr_window;
        window->rect.w += window->padding;

        if (window->rect.h <= 1) window->rect.h += (height + 2*window->padding);
        if (window->rect.h <= (height + 2*window->padding))
            window->rect.h += ((height + 2*window->padding) - window->rect.h);

        btn_rect.left = window->rect.left + window->rect.w;
        btn_rect.top  = window->rect.top  + window->padding;

        window->rect.w += width;
    }


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

    const i32 BTN_Z_INDEX = -2;
    /* push elements to render */
    ctx->render_buf[ctx->render_elems_count++] = {nullptr, {0}, btn_rect, BTN_Z_INDEX, btn_color};
    if (sprite)
    {
        // try to center the sprite in the button
        i32 sprite_offset_x = (width  - sprite->box.w)/2;
        i32 sprite_offset_y = (height - sprite->box.h)/2;
        i32 sprite_start_x  = btn_rect.left + sprite_offset_x;
        i32 sprite_start_y  = btn_rect.top  + sprite_offset_y;
        rect_t dst = {sprite_start_x, sprite_start_y, sprite->box.w, sprite->box.h};

        ctx->render_buf[ctx->render_elems_count++] = {sprite->tex, sprite->box, dst, BTN_Z_INDEX-1};
    }

    /* push font chars to render */
    if (text)
    {
        // determine width of text
        u32 text_len = strlen(text);
        u32 text_width_px  = text_len * ctx->font.FONT_CHAR_WIDTH;
        u32 text_height_px = ctx->font.FONT_CHAR_HEIGHT; // TODO take multiline text into account

        // determine if we need to insert a linebreak
        // text_width is larger than button width
        //if (text_width_px > width)

        // if (centered)
            i32 text_offset_x = (width  - text_width_px)/2;
            i32 text_offset_y = (height - text_height_px)/2;
            i32 text_start_x  = btn_rect.left + text_offset_x;
            i32 text_start_y  = btn_rect.top  + text_offset_y;

        // determine height of text
        // are there newlines in the text?

        rect_t char_dst = {text_start_x, text_start_y, ctx->font.FONT_CHAR_WIDTH, ctx->font.FONT_CHAR_HEIGHT};
        for (int i = 0; i < text_len; i++)
        {
            if (text[i] == '\n')
            {
                char_dst.top  += ctx->font.FONT_CHAR_HEIGHT;
                char_dst.left  = text_start_x;
                continue;
            }

            rect_t char_box = ui_font_get_box_for_char(ctx->font, text[i]);
            //rect_t char_box = {12,12, ctx->font.FONT_CHAR_WIDTH, ctx->font.FONT_CHAR_HEIGHT};
            ctx->render_buf[ctx->render_elems_count++] = {ctx->font.bitmap, char_box, char_dst, BTN_Z_INDEX-2};
            char_dst.left += char_box.w;
        }
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
