#pragma once

#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"

class Torch : public Actor
{
private:
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;

    DrawAnimatedComponent* mDrawComponent;

public:
    Torch(
        Game *game, 
        const Vector2& center
    );
};