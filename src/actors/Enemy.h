#pragma once

#include "Actor.h"
#include <SDL.h>

class Enemy : public Actor
{
public:
    explicit Enemy(Game* game, float forwardSpeed, const Vector2& position, float fowardSpeed=1000.f);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void ManageState();
    std::vector<Vector2> GetPath() const;

private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
    class AIMovementComponent* mAIMovementComponent;

    void ManageAnimations();
    void TakeDamage();
    void AnimationEndCallback(std::string animationName);

    bool PlayerOnSight();
    bool PlayerOnFov();
};