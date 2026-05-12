#pragma once
#include "Actor.h"
#include <SDL.h>

class Star : public Actor
{
public:
    explicit Star(Game* game);
    void OnUpdate(float deltaTime) override;
    void ManageState();

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override {};
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override {};

private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
};