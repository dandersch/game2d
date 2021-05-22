#include "input.h"

u32 Input::actionState = 0;
//b8 actionState[ACTION_COUNT] = {0};

void Input::update()
{
    actionState = 0;

    // TODO use inputhandler
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W]) actionState |= ACTION_MOVE_UP;
    if (keystate[SDL_SCANCODE_A]) actionState |= ACTION_MOVE_LEFT;
    if (keystate[SDL_SCANCODE_S]) actionState |= ACTION_MOVE_DOWN;
    if (keystate[SDL_SCANCODE_D]) actionState |= ACTION_MOVE_RIGHT;

    if (keystate[SDL_SCANCODE_UP]   ) actionState |= ACTION_MOVE_UP;
    if (keystate[SDL_SCANCODE_DOWN] ) actionState |= ACTION_MOVE_DOWN;
    if (keystate[SDL_SCANCODE_LEFT] ) actionState |= ACTION_MOVE_LEFT;
    if (keystate[SDL_SCANCODE_RIGHT]) actionState |= ACTION_MOVE_RIGHT;
}
