

// eventsystem

// camera

// animation

// collisions

// audiosystem

// playercontroller

// tilesystem

// resource/assetmanager

#ifdef false

class GameState {

entities[MAX_AMOUNT_OF_ENTITIES];
entityCount;
entityIdx;

entities[getIdxForNewEntity()] = { .position = {},
                   .sfx      =   ,
                   .anim     =   ,
                   .sprite   =   ; }

int getIdxForNewEntity()
{
    for (ent : entities)
    {
        if (ent.freed) return idx;
    }
}


void update(f32 dt)
{
    // TODO map from keys to commands
    inputmanager.update();

    for (ent : entites)
    {
        if (!ent.active) continue;

        if (ent.flags & PLAYER_CONTROLLED)
            player.control(dt, ent); // also records commands

        if (ent.flags & IS_ITEM)
            itemmanager.update(dt, ent); // updates attached items, maybe also combines

        if (ent.flags & REPLAY_ACTIVATED)
            commandProcessor.update(dt, ent);

        if (ent.flags & ENEMY)
            aicontroller.update(dt, ent);

        if (ent.flags & ANIMATED)
            animator.update(dt, ent);

        if (ent.flags & TIME_REWINDABLE)
            timerewinder.update(dt, ent); // records/rewinds
    }
}

void fixedUpdate(f32 dt)
{
    if (ent.flags & PHYSICS_BODY)
        physics.update(dt, ent);
}

#endif
