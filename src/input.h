#pragma once

enum Action
{
    ACTION_MOVE_UP    = (1 << 0),
    ACTION_MOVE_DOWN  = (1 << 1),
    ACTION_MOVE_RIGHT = (1 << 2),
    ACTION_MOVE_LEFT  = (1 << 3),
    ACTION_PICKUP     = (1 << 4),
    ACTION_ATTACK     = (1 << 5),
    ACTION_RESTART    = (1 << 6),
    ACTION_COUNT      = 7
};

namespace Input
{
    //extern u32 actionState;
    //b8 actionState[ACTION_COUNT] = {0};
    void update();

} // namespace Input

struct game_input_state_t
{
    i32 up_down_count; // count of up/down or down/up transitions
    b32 is_down; // NOTE indicates that input was down when last frame ended,
                 // not that it is necessarily down at the moment
};

struct game_keyboard_input_t
{
    game_input_state_t keys[128]; // NOTE index into this with a char/u8
    b32 shift_down, alt_down, ctrl_down;

    // TODO up/down/left/right keys
    game_input_state_t key_up;
    game_input_state_t key_down;
    game_input_state_t key_left;
    game_input_state_t key_right;

    // NOTE For debugging only
    b32 f_key_pressed[13]; // NOTE 1 is F1, etc., for clarity - 0 is not used!
};

enum game_input_mouse_button
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_EXTENDED0,
    MOUSE_BUTTON_EXTENDED1,
    MOUSE_BUTTON_COUNT,
};

struct game_mouse_input_t
{
    game_input_state_t buttons[MOUSE_BUTTON_COUNT];
    v3i pos; // NOTE in clip space
    // TODO mousewheel input
};

struct game_pad_input_t
{
    b32 connected;
    b32 analog;
    f32 stick_avg_x;
    f32 stick_avg_y;
    f32 clutch_max; // NOTE(casey): This is the "dodge" clutch, eg. triggers or space bar?

    union
    {
        game_input_state_t buttons[12];
        struct
        {
            game_input_state_t move_up;
            game_input_state_t move_down;
            game_input_state_t move_left;
            game_input_state_t move_right;

            game_input_state_t action_up;
            game_input_state_t action_down;
            game_input_state_t action_left;
            game_input_state_t action_right;

            game_input_state_t left_shoulder;
            game_input_state_t right_shoulder;

            game_input_state_t back;
            game_input_state_t start;

            // NOTE(casey): All inputs must be added above this line

            game_input_state_t terminator; // TODO what is this used for
        };
    };
};

#define MAX_CONTROLLER_COUNT 5
struct game_input_t
{
    f32 dt_for_frame; // TODO what is this

    game_pad_input_t        pads[MAX_CONTROLLER_COUNT];
    game_keyboard_input_t   keyboard;
    game_mouse_input_t      mouse;

    b32 quit_requested; // NOTE signals back to the platform layer
};


inline b32 input_pressed(game_input_state_t state)
{
    return ((state.up_down_count > 1)
            || ((state.up_down_count == 1) && (state.is_down)));
}

inline b32 input_down(game_input_state_t state) { return (state.is_down); }

// TODO not used
inline game_pad_input_t* gamepad_get(game_input_t* input, int unsigned gamepad_idx)
{
    ASSERT(gamepad_idx < (sizeof(input->pads)/ sizeof((input->pads)[0])));
    return &input->pads[gamepad_idx];
}
