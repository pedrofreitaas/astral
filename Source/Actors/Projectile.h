#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(class Game* game, ColliderLayer layer, int type=1);

    void OnUpdate(float deltaTime) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:    
    class DrawSpriteComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};