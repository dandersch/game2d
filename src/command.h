#pragma once

#include "pch.h"

struct Entity;

class Command
{
public:
    Command() = default;
    virtual void execute(Entity& e) {}
    //virtual void undo(); // NOTE possibly never needed
};

class MoveCommand : public Command
{
public:
    MoveCommand(glm::vec3 movement);
    virtual void execute(Entity& e) override;

private:
    glm::vec3 move;
};

namespace CommandProcessor
{

// TODO use move semantics?
void record(Entity& ent, Command* cmd);
void replay(Entity& ent);

} // namespace CommandProcessor