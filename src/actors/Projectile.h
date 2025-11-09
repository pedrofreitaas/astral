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
        Vector2 target, float speed
    );

protected:
    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    virtual void ManageAnimations() = 0;
    void Kill();
    
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class DrawAnimatedComponent* mDrawAnimatedComponent;
    class TimerComponent* mTimerComponent;
    
    Vector2 mTarget, mDirection;
    float mKnockbackIntensity;
    float mSpeed;
};
