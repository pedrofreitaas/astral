#pragma once

#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/RigidBodyComponent.h"

class Father : public Actor
{
private:
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;
    DrawAnimatedComponent* mDrawComponent;
    AABBColliderComponent* mColliderComponent;
    RigidBodyComponent* mRigidBodyComponent;

public:
    Father(
        Game *game, 
        const Vector2& center
    );
};