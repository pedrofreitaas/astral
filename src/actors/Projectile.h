#pragma once
#include <SDL.h>
#include "../core/Game.h"
#include "Actor.h"

class Projectile : public Actor
{
    float MAX_DIE_TIME = 3.f;
    
public:
    Projectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    );

protected:
    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    virtual void ManageAnimations() = 0;
    virtual void Kill();
    
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class DrawAnimatedComponent* mDrawAnimatedComponent;
    class TimerComponent* mTimerComponent;
    
    Vector2 mDirection;
    float mKnockbackIntensity;
    float mSpeed;
};
