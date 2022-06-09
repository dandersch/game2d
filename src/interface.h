#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "utils.h"

typedef i32 ui_id;

// TODO embed a bitmap font
//#include "bitmap_font.h"

/* common operations */
inline static f32 lerp(f32 min, f32 interpolant, f32 max) { return (min + interpolant * (max - min)); }
inline static f32 unlerp(f32 min, f32 value, f32 max) { return (min != max) ? (value - min)/(max - min) : 0.0f; }
inline static f32 linear_remap(f32 val, f32 a_min, f32 a_max, f32 b_min, f32 b_max) { return lerp(b_min, unlerp(a_min, val, a_max), b_max);}

#define PUSH_TEXTURE(tex, src, dst, z)                                                    \
{                                                                                         \
    ctx->texture_buf[ctx->texture_count++] = {tex, src, dst, z, {RENDER_ENTRY_FLAG_UI}};  \
}

#define PUSH_RECT(dst, z, color)                                                \
{                                                                               \
    ctx->rect_buf[ctx->rect_count++] = {dst, z, color, {RENDER_ENTRY_FLAG_UI}}; \
}

#define UI_ELEMENTS_MAX 1000 // NOTE right now every character is an 'element'
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

    UI_COLOR_SLIDER,
    UI_COLOR_SLIDER_LEFT,
    UI_COLOR_SLIDER_RIGHT,

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
    WINDOW_LAYOUT_HORIZONTAL,
    WINDOW_LAYOUT_VERTICAL,

    WINDOW_LAYOUT_HORIZONTAL_UP, // not working
    WINDOW_LAYOUT_VERTICAL_LEFT, // not working

    // WINDOW_LAYOUT_CENTERED_HORIZONTAL
    // WINDOW_LAYOUT_CENTERED_VERTICALLY
    // WINDOW_LAYOUT_CENTERED

    WINDOW_LAYOUT_COUNT
};
struct ui_window_t
{
    b32 active;
    rect_t rect;
    ui_id  id;
    u32 style = WINDOW_LAYOUT_VERTICAL;
    i32 zindex  = -3; // smallest zindex gets drawn last
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
    b32 window_focused = false; // TODO just have one general "ui_focused" bool
                                // that we can check to see if we should let an event through to the game

    /* rendering */
    render_entry_texture_t texture_buf[UI_ELEMENTS_MAX]; // used by icons & text glyphs
    render_entry_rect_t    rect_buf[UI_ELEMENTS_MAX];    // windows,buttons, etc.
    u32 texture_count;
    u32 rect_count;

    Camera cam; // TODO workaround bc of orthographic cam

    /* unused */
    ui_id id_stack[UI_ELEMENTS_MAX]; // TODO per frame to check for id collisions
};


inline void ui_init(ui_t* ctx, renderer_api_t* renderer)
{
    ctx->style.colors[UI_COLOR_BUTTON]       = {0.6f, 0.2f, 0.2f, 1.0f};
    ctx->style.colors[UI_COLOR_BUTTONHOVER]  = {0.8f, 0.2f, 0.2f, 1.0f};
    ctx->style.colors[UI_COLOR_WINDOW]       = {0.2,0.2,1.0,1.0};
    ctx->style.colors[UI_COLOR_SLIDER]       = {0.1,0.8,1.0,1.0};
    ctx->style.colors[UI_COLOR_SLIDER_LEFT]  = {0.4,0.1,1.0,0.9};
    ctx->style.colors[UI_COLOR_SLIDER_RIGHT] = {0.1,0.1,0.5,0.9};
    ctx->font.bitmap = resourcemgr_texture_load("charmap-cellphone_white-transparent.png", renderer);
}


inline void ui_begin(ui_t* ctx)
{
    /* TODO assert */
    ctx->texture_count = 0;
    ctx->rect_count    = 0;
    ctx->curr_window.active = 0;
    ctx->window_focused     = false;
    ctx->curr_focus         = __COUNTER__; // zero, TODO better id system
}


inline void ui_end(ui_t* ctx)
{
    //ASSERT(ctx->curr_window.active == false); // forgot to call ui_window_end?
    //ctx->curr_window.active = 0; // NOTE also in ui_begin...
}


inline void ui_window_begin(ui_t* ctx, i32 left, i32 top, ui_id id, u32 style = WINDOW_LAYOUT_VERTICAL)
{
    // TODO making the window 0 by 0 big makes it fill the entire screen right now
    ctx->curr_window = {1, {left, top, 1, 1}, id, style};
}


inline void ui_window_end(ui_t* ctx)
{
    // TODO for centering: we could calculate an offset
    //   centered_x = (SCREEN_WIDTH  - window.width)  / 2
    //   centered_y = (SCREEN_HEIGHT - window.height) / 2
    //   offset_x   = window.left - centered_x;
    //   offset_y   = window.top - centered_y;
    // and apply it to all vertices here
    //
    // in fact, if we do this, we can just let the user supply a position on the
    // screen that describes the center of the window

    switch (ctx->curr_window.style) // add last bit of padding
    {
        case WINDOW_LAYOUT_HORIZONTAL: { ctx->curr_window.rect.h += ctx->curr_window.padding; } break;
        case WINDOW_LAYOUT_VERTICAL:   { ctx->curr_window.rect.w += ctx->curr_window.padding; } break;
    }

    if (utils_point_in_rect(ctx->mouse_pos, ctx->curr_window.rect))
        ctx->window_focused = true;

    rect_t   dst   = ctx->curr_window.rect;
    i32      z_idx = ctx->curr_window.zindex;
    colorf_t color = ctx->style.colors[UI_COLOR_WINDOW];
    PUSH_RECT(dst, z_idx, color);

    // if (skeuomorphic) {
        rect_t   dst_bg   = dst;
        dst_bg.left += 7;
        dst_bg.top  += 7;
        colorf_t color_bg = color;
        color_bg.r -= 0.1f;
        color_bg.g -= 0.1f;
        color_bg.b -= 0.1f;
        color_bg.a += 0.2f;
        PUSH_RECT(dst_bg, z_idx+1, color_bg);
    // }

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


/*internal_fn*/
inline void bump_window_size_by_rect(ui_window_t* window, rect_t* rect_to_add)
{
    // bump window size by padding + button size depending on window style
    switch (window->style)
    {
        case WINDOW_LAYOUT_HORIZONTAL:
        {
            window->rect.h += window->padding;

            if (window->rect.w <= 1) window->rect.w += (rect_to_add->w + 2*window->padding);
            if (window->rect.w <= (rect_to_add->w  + 2*window->padding))
                window->rect.w += ((rect_to_add->w + 2*window->padding) - window->rect.w);

            rect_to_add->top  = window->rect.top  + window->rect.h;
            rect_to_add->left = window->rect.left + window->padding;

            window->rect.h += rect_to_add->h;
        } break;
        case WINDOW_LAYOUT_HORIZONTAL_UP: // window grows upwards
        {
            window->rect.h   -= window->padding;

            if (window->rect.w <= 1) window->rect.w += (rect_to_add->w + 2*window->padding);
            if (window->rect.w <= (rect_to_add->w  + 2*window->padding))
                window->rect.w += ((rect_to_add->w + 2*window->padding) - window->rect.w);

            rect_to_add->top  = window->rect.top  + window->rect.h;
            rect_to_add->left = window->rect.left + window->padding;

            window->rect.h -= rect_to_add->h;
        } break;
        case WINDOW_LAYOUT_VERTICAL:
        {
            window->rect.w += window->padding;

            if (window->rect.h <= 1) window->rect.h += (rect_to_add->h + 2*window->padding);
            if (window->rect.h <= (rect_to_add->h + 2*window->padding))
                window->rect.h += ((rect_to_add->h + 2*window->padding) - window->rect.h);

            rect_to_add->left = window->rect.left + window->rect.w;
            rect_to_add->top  = window->rect.top  + window->padding;

            window->rect.w += rect_to_add->w;
        } break;
    }
}


// TODO maybe add & use a float rect_t
// TODO add an optional animator that updates the sprite when hovering over the button
inline b32 ui_button(ui_t* ctx, i32 width, i32 height, sprite_t* sprite, ui_id id, const char* text = nullptr)
{
    ASSERT(ctx->curr_window.active); // window now required

    colorf_t btn_color = ctx->style.colors[UI_COLOR_BUTTON];
    rect_t   btn_rect  = {0,0, width, height};

    bump_window_size_by_rect(&ctx->curr_window, &btn_rect);

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

    const i32 BTN_Z_INDEX = -5;
    /* push elements to render */
    PUSH_RECT(btn_rect, BTN_Z_INDEX, btn_color);
    if (sprite)
    {
        // try to center the sprite in the button
        i32 sprite_offset_x = (width  - sprite->box.w)/2;
        i32 sprite_offset_y = (height - sprite->box.h)/2;
        i32 sprite_start_x  = btn_rect.left + sprite_offset_x;
        i32 sprite_start_y  = btn_rect.top  + sprite_offset_y;
        rect_t dst = {sprite_start_x, sprite_start_y, sprite->box.w, sprite->box.h};

        PUSH_TEXTURE(sprite->tex, sprite->box, dst, BTN_Z_INDEX-1);
    }

    /* push font chars to render */
    if (text)
    {
        u32 text_len = strlen(text);

        // count newlines & find longest line
        u32 line_count              = 1;
        u32 longest_line_char_count = 0;
        u32 character_count = 0;
        for (int i = 0; i < text_len; i++)
        {
            character_count++;
            if (text[i] == '\n')
            {
                line_count++;
                character_count = 0;
            }
            if (character_count > longest_line_char_count)
                longest_line_char_count = character_count;
        }
        if (line_count == 1) longest_line_char_count = text_len;

        // determine width of text
        i32 width_per_char  = ctx->font.FONT_CHAR_WIDTH;
        i32 height_per_char = ctx->font.FONT_CHAR_HEIGHT;
        u32 text_width_px  = longest_line_char_count * width_per_char;
        u32 text_height_px = height_per_char * line_count; // TODO take multiline text into account

        // see if the text fits inside the rect & if it does, try to make the text bigger
        {
            // determine if we need to insert a linebreak
            // text_width is larger than button width
            //if (text_width_px > width)
            // TODO consider if (btn_rect.height < text_height_px)
            /* while ((btn_rect.w-10) > text_width_px) */
            /* { */
            /*     width_per_char  += 1; */
            /*     height_per_char += 2; */
            /*     text_width_px  = longest_line_char_count * width_per_char; */
            /*     text_height_px = height_per_char * line_count; */
            /* } */
        }

        // if (centered) {
            i32 text_offset_x = (width  - text_width_px)/2;
            i32 text_offset_y = (height - text_height_px)/2;
            i32 text_start_x  = btn_rect.left + text_offset_x;
            i32 text_start_y  = btn_rect.top  + text_offset_y;
        // }

        // determine height of text
        // are there newlines in the text?

        rect_t char_dst = {text_start_x, text_start_y, width_per_char, height_per_char};
        for (int i = 0; i < text_len; i++)
        {
            if (text[i] == '\n')
            {
                char_dst.top  += height_per_char;
                char_dst.left  = text_start_x;
                continue;
            }

            rect_t char_box = ui_font_get_box_for_char(ctx->font, text[i]);
            PUSH_TEXTURE(ctx->font.bitmap, char_box, char_dst, BTN_Z_INDEX-2);

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


// TODO better colors
// TODO make draggable
inline b32 ui_slider_float(ui_t* ctx, f32* value, ui_id id, f32 min = 0.0f, f32 max = 1.0f,
                           i32 width = 100, i32 height = 20/*, const char* text = nullptr*/)
{
    ASSERT(ctx->curr_window.active); // window now required

    colorf_t slider_color = ctx->style.colors[UI_COLOR_SLIDER];
    colorf_t widget_color = ctx->style.colors[UI_COLOR_SLIDER_RIGHT];
    colorf_t left_color   = ctx->style.colors[UI_COLOR_SLIDER_LEFT];
    rect_t widget_rect  = {0,0, width, height};

    bump_window_size_by_rect(&ctx->curr_window, &widget_rect);

    f32 normalized_value = (*value - min)/(max - min); // bring value in 0-1 range

    // determine where the slider is
    i32 slider_padding_px  = 6;  // make the slider slightly taller than the widget
    i32 slider_width       = 10; // width of the actual slider "button"
    i32 slider_pos_x       = widget_rect.left + (((normalized_value) * width) - (slider_width/2));
    rect_t slider_rect     = {slider_pos_x, widget_rect.top - (slider_padding_px/2),
                              slider_width, height + slider_padding_px};
    rect_t left_rect       = {widget_rect.left, widget_rect.top,
                              (slider_rect.left - widget_rect.left), widget_rect.h};
    //rect_t right_rect      = {0,0, width, height};

    // slider logic
    f32 new_value_if_pressed = *value;
    if (utils_point_in_rect(ctx->mouse_pos, widget_rect))
    {
        ctx->curr_focus = id;

        // slider "click" logic
        i32 delta_px = ctx->mouse_pos.x - widget_rect.left;
        f32 interpolant = (f32) delta_px / (f32) widget_rect.w;
        new_value_if_pressed = LERP(min, max, interpolant);
    }

    b32 pressed = false;
    if (ctx->curr_focus == id)
    {
        slider_color = ctx->style.colors[UI_COLOR_BUTTONHOVER];
        if (ctx->mouse_pressed)
        {
            if (ctx->curr_focus == id)
            {
                pressed = true;
                *value = new_value_if_pressed;
            }
        }
    }

    const i32 BTN_Z_INDEX = -5;
    /* push elements to render */
    PUSH_RECT(widget_rect, BTN_Z_INDEX, widget_color);
    PUSH_RECT(left_rect,   BTN_Z_INDEX, left_color);
    PUSH_RECT(slider_rect, BTN_Z_INDEX - 1, slider_color);

    return pressed;
}

inline void ui_icon(ui_t* ctx, ui_id id, sprite_t sprite, f32 size = 1)
{
    rect_t sprite_rect = {0, 0, (i32) (sprite.box.w * size), (i32) (sprite.box.h * size)};
    bump_window_size_by_rect(&ctx->curr_window, &sprite_rect);

    //i32 sprite_offset_x = (width  - sprite->box.w)/2;
    //i32 sprite_offset_y = (height - sprite->box.h)/2;
    i32 sprite_start_x  = sprite_rect.left ;
    i32 sprite_start_y  = sprite_rect.top  ;
    rect_t dst = {sprite_start_x, sprite_start_y, sprite_rect.w, sprite_rect.h};

    PUSH_TEXTURE(sprite.tex, sprite.box, dst, -6);
}

// TODO support sprintf
inline void ui_text(ui_t* ctx, ui_id id, const char* text, u32 font_size = 1)
{
    ASSERT(ctx->curr_window.active);

    // TODO duplicated from ui_button
    u32 text_len = strlen(text);

    // count newlines & find longest line
    u32 line_count              = 1;
    u32 longest_line_char_count = 0;
    u32 character_count = 0;
    for (int i = 0; i < text_len; i++)
    {
        character_count++;
        if (text[i] == '\n')
        {
            line_count++;
            character_count = 0;
        }
        if (character_count > longest_line_char_count)
            longest_line_char_count = character_count;
    }
    if (line_count == 1) longest_line_char_count = text_len;

    // determine width of text
    i32 width_per_char  = ctx->font.FONT_CHAR_WIDTH * font_size;
    i32 height_per_char = ctx->font.FONT_CHAR_HEIGHT * font_size;
    i32 text_width_px  = longest_line_char_count * width_per_char;
    i32 text_height_px = height_per_char * line_count;

    rect_t text_rect = {0,0, text_width_px, text_height_px};
    bump_window_size_by_rect(&ctx->curr_window, &text_rect);

    i32 text_start_x  = text_rect.left;
    i32 text_start_y  = text_rect.top;

    rect_t char_dst = {text_start_x, text_start_y, width_per_char, height_per_char};
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == '\n')
        {
            char_dst.top  += height_per_char;
            char_dst.left  = text_start_x;
            continue;
        }

        rect_t char_box = ui_font_get_box_for_char(ctx->font, text[i]);
        PUSH_TEXTURE(ctx->font.bitmap, char_box, char_dst, -10);
        char_dst.left += width_per_char;
    }
}

inline void ui_render(ui_t* ctx, renderer_api_t renderer, Camera cam)
{
    ASSERT(ctx->rect_count    < UI_ELEMENTS_MAX);
    ASSERT(ctx->texture_count < UI_ELEMENTS_MAX);

    for (u32 i = 0; i < ctx->rect_count; i++)
    {
        renderer.push_rect(ctx->rect_buf[i]);
    }

    for (u32 i = 0; i < ctx->texture_count; i++)
    {
        renderer.push_texture(ctx->texture_buf[i]);
    }

    f32 left   = 0; f32 top    = 0;
    f32 right  = SCREEN_WIDTH; f32 bottom = SCREEN_HEIGHT;
    f32 near   = -50; f32 far    = 50;

    // TODO add an m4f_orthographic() function
    m4f ui_mtx = {{ {2/(right-left),              0,               0, 0},
                  {             0, 2/(top-bottom),               0, 0},
                  {             0,              0, (2)/(far-near), 0},
                  {-((right+left)/(right-left)),-((top+bottom)/(top-bottom)), -((far+near)/(far-near)), 1}}};
    RENDERER_PUSH_UI_MTX(ui_mtx);
}

#undef PUSH_TEXTURE
#undef PUSH_RECT
#endif // INTERFACE_H_
