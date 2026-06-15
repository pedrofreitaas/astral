#pragma once

#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/RigidBodyComponent.h"

class MetalCrate : public Actor
{
private:
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;
    void AnimationEndCallback(std::string animationName);

    DrawAnimatedComponent* mDrawComponent;
    RigidBodyComponent* mRigidBodyComponent;
    AABBColliderComponent* mColliderComponent;

    void Freeze() override;
    void StopFreeze() override;

    bool IsInInvalidPosition();

    Vector2 mSpawnCenter;

public:
    MetalCrate(
        Game *game, 
        const Vector2& center
    );
};