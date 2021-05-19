#include "player.h"

#include "collision.h"
#include "gamelayer.h"
#include "entity.h"

void Player::handleEvent(const Event& e,  Entity& ent)
{
    SDL_Event evn = e.evn;

    switch (evn.type) {
    case SDL_MOUSEBUTTONDOWN:
        ent.position = glm::vec3(evn.button.x, evn.button.y, 0);
        break;
    }
}

void Player::update(f32 dt, Entity &ent)
{
    // TODO we shouldnt move entities in here, just record the input
    glm::vec3 movement = {0,0,0};

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W])
        movement = glm::vec3( 0,-1,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_A])
        movement = glm::vec3(-1, 0,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_S])
        movement = glm::vec3( 0, 1,0) * playerSpeed * dt;
    if (keystate[SDL_SCANCODE_D])
        movement = glm::vec3( 1, 0,0) * playerSpeed * dt;

    tryMove(movement, ent);
}

void Player::tryMove(glm::vec3 movement, Entity& ent)
{
    Orientation newOrient = (Orientation) ent.orient;
    if      (movement.y < 0.0f) newOrient = Orientation::UP;
    else if (movement.y > 0.0f) newOrient = Orientation::DOWN;
    else if (movement.x > 0.0f) newOrient = Orientation::RIGHT;
    else if (movement.x < 0.0f) newOrient = Orientation::LEFT;
    EntityState newState = (EntityState) ent.state;
    if (movement != glm::vec3{0,0,0})  newState = EntityState::MOVE;
    else newState = EntityState::IDLE;

    //ChangeAnimationState(newAnim, newOrient);
    ent.state  = newState;
    if (ent.orient != (u32) newOrient)
    {
        ent.orient = (u32) newOrient;
        ent.anim   = ent.anims[ent.orient]; // TODO support states
    }

    // temp. collision code
    // TODO variable dt creates tunneling effect
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (!GameLayer::ents[i].active) continue;
        if ((GameLayer::ents[i].flags & (u32) EntityFlag::IS_COLLIDER) &&
            &GameLayer::ents[i] != &ent)
        {
            SDL_Rect a = {(i32) (ent.position.x + movement.x) + ent.collider.x,
                          (i32) (ent.position.y + movement.y) + ent.collider.y,
                          ent.collider.w, ent.collider.h,} ;
            const auto& entB = GameLayer::ents[i];
            SDL_Rect b = {(i32) entB.position.x + entB.collider.x,
                          (i32) entB.position.y + entB.collider.y,
                          entB.collider.w, entB.collider.h,} ;

            if (Collision::AABB(a, b))
            {
                //printf("COLLISION\n");
                movement = {0,0,0};
            }
        }
    }

    ent.position += movement;
}
