#pragma once

#include "Actor.h"
#include <SDL.h>

class Enemy : public Actor
{
public:
    explicit Enemy(Game* game, float forwardSpeed, const Vector2& position);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void ManageState();

private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    void ManageAnimations();
    void TakeDamage();
};