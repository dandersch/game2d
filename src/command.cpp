#include "command.h"
#include "entity.h"

#define MAX_CMD_COUNT 10000

#include "memory.h"
extern game_state_t* state;

// TODO use move semantics?
void CommandProcessor::record(Entity& ent, Command cmd)
{
    ASSERT(cmdIdx <= MAX_CMD_COUNT - 1);

    ent.cmds[state->cmdIdx] = cmd;
    execute(ent, ent.cmds[state->cmdIdx]);
}

void CommandProcessor::replay(Entity& ent)
{
    ASSERT(cmdIdx <= MAX_CMD_COUNT - 1);

    execute(ent, ent.cmds[state->cmdIdx]);
}

// TODO use move semantics?
void CommandProcessor::execute(Entity& ent, Command cmd)
{
    switch (cmd.type)
    {
    case Command::MOVE:
        player_try_move(cmd.movement, ent);
        break;

    case Command::PICKUP:
        player_try_pickup(cmd.movement, ent);
        break;

    case Command::ATTACK:
        player_try_attack(cmd.movement, ent);
        break;

    default:
        break;
    }
}

void CommandProcessor::initialize(Entity& ent)
{
    ent.cmds = new Command[MAX_CMD_COUNT];
}

void CommandProcessor::onEndUpdate()
{
    state->cmdIdx++;
}
