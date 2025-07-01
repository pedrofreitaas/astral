#include "PunkArm.h"

PunkArm::PunkArm(Game* game, Punk* punk)
    : Actor(game), mPunk(punk)
{
    mDrawComponent = new DrawSpriteComponent(
        this, 
        "../Assets/Sprites/Punk/arm_gun.png", 
        18, 28, 
        200
    );
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));
}