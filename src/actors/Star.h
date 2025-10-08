#pragma once
#include "Actor.h"
#include <SDL.h>

class Star : public Actor
{
public:
    explicit Star(Game* game);

    Vector2 GetCenter() const
    {
        return mColliderComponent->GetCenter();
    }

private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};