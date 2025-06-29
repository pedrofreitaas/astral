#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(class Game* game, float length = 10.0f, float deathTimer = 1.0f, ColliderLayer layer = ColliderLayer::EnemyProjectile);

    void OnUpdate(float deltaTime) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:
    float mLength;
    float mDeathTimer;

    class DrawSpriteComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};