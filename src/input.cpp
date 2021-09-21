#include "input.h"

u32 Input::actionState = 0;

// Note on SDL_GetKeyboardState():
// This function gives you the current state after all events have been
// processed, so if a key or button has been pressed and released before you
// process events, then the pressed state will never show up in the
// SDL_GetKeyboardState() calls.
//
// This function doesn't take into account whether shift has been pressed
// or not.

#include "globals.h"
void Input::update()
{
    actionState = 0;

    //const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (input_down(globals.game_input.keyboard.keys['w'])) actionState |= ACTION_MOVE_UP;
    if (input_down(globals.game_input.keyboard.keys['a'])) actionState |= ACTION_MOVE_LEFT;
    if (input_down(globals.game_input.keyboard.keys['s'])) actionState |= ACTION_MOVE_DOWN;
    if (input_down(globals.game_input.keyboard.keys['d'])) actionState |= ACTION_MOVE_RIGHT;

    //if (keystate[SDL_SCANCODE_UP]   ) actionState |= ACTION_MOVE_UP;
    //if (keystate[SDL_SCANCODE_DOWN] ) actionState |= ACTION_MOVE_DOWN;
    //if (keystate[SDL_SCANCODE_LEFT] ) actionState |= ACTION_MOVE_LEFT;
    //if (keystate[SDL_SCANCODE_RIGHT]) actionState |= ACTION_MOVE_RIGHT;

    if (input_pressed(globals.game_input.keyboard.keys['r'])) actionState |= ACTION_RESTART;
    if (input_pressed(globals.game_input.keyboard.keys['f'])) actionState |= ACTION_PICKUP;
    if (input_pressed(globals.game_input.keyboard.keys['e'])) actionState |= ACTION_ATTACK;
}
