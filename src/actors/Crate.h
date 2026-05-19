#pragma once

#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"

class Crate : public Actor
{
private:
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;
    void AnimationEndCallback(std::string animationName);

    DrawAnimatedComponent* mDrawComponent;

public:
    Crate(
        Game *game, 
        const Vector2& center
    );
};