#include "command.h"
#include "entity.h"
#include "player.h"

MoveCommand::MoveCommand(glm::vec3 movement) : move(movement) {}

void MoveCommand::execute(Entity& e)
{
    Player::tryMove(move, e);
}

// TODO use move semantics?
void CommandProcessor::record(Entity& ent, Command* cmd)
{
    ASSERT(ent.cmdIdx <= MAX_CMD_COUNT - 1);

    ent.cmds[ent.cmdIdx] = cmd;
    ent.cmds[ent.cmdIdx]->execute(ent); // NOTE we execute after recording
    ent.cmdIdx++;
}

void CommandProcessor::replay(Entity& ent)
{
    ASSERT(ent.cmdIdx <= MAX_CMD_COUNT - 1);

    if (ent.cmds[ent.cmdIdx] != nullptr)
        ent.cmds[ent.cmdIdx]->execute(ent);
    ent.cmdIdx++;
}
