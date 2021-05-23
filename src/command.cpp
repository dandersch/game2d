#include "command.h"
#include "entity.h"

#define MAX_CMD_COUNT 10000

u32 CommandProcessor::cmdIdx = 0;

// TODO use move semantics?
void CommandProcessor::record(Entity& ent, Command cmd)
{
    ASSERT(cmdIdx <= MAX_CMD_COUNT - 1);

    ent.cmds[cmdIdx] = cmd;
    execute(ent, ent.cmds[cmdIdx]);
}

void CommandProcessor::replay(Entity& ent)
{
    ASSERT(cmdIdx <= MAX_CMD_COUNT - 1);

    execute(ent, ent.cmds[cmdIdx]);
}

// TODO use move semantics?
void CommandProcessor::execute(Entity& ent, Command cmd)
{
    switch (cmd.type)
    {
    case Command::MOVE:
        Player::tryMove(cmd.movement, ent);
        break;

    case Command::PICKUP:
        Player::tryPickUp(cmd.movement, ent);
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
    cmdIdx++;
}
