#include "input.h"

#include "memory.h"
extern game_state_t* state;

// Note on SDL_GetKeyboardState():
// This function gives you the current state after all events have been
// processed, so if a key or button has been pressed and released before you
// process events, then the pressed state will never show up in the
// SDL_GetKeyboardState() calls.
//
// This function doesn't take into account whether shift has been pressed
// or not.

void Input::update()
{
    state->actionState = 0;

    if (input_down(state->game_input.keyboard.keys['w'])) state->actionState |= ACTION_MOVE_UP;
    if (input_down(state->game_input.keyboard.keys['a'])) state->actionState |= ACTION_MOVE_LEFT;
    if (input_down(state->game_input.keyboard.keys['s'])) state->actionState |= ACTION_MOVE_DOWN;
    if (input_down(state->game_input.keyboard.keys['d'])) state->actionState |= ACTION_MOVE_RIGHT;

    if (input_pressed(state->game_input.keyboard.keys['r'])) state->actionState |= ACTION_RESTART;
    if (input_pressed(state->game_input.keyboard.keys['f'])) state->actionState |= ACTION_PICKUP;
    if (input_pressed(state->game_input.keyboard.keys['e'])) state->actionState |= ACTION_ATTACK;
}
