#pragma once

#include "Actor.h"
#include <SDL.h>

class Portal : public Actor
{
public:
    explicit Portal(Game* game, const Vector2& position);

private:
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};