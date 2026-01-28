#pragma once
#include <SDL.h>
#include "../core/Game.h"
#include "Actor.h"

class Projectile : public Actor
{    
public:
    Projectile(
        class Game* game, Vector2 position,
        Actor* shooter, float mDieTime=3.f
    );

private:
    class TimerComponent* mTimerComponent;

protected:
    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    // Should be called at the end of the subclass constructor
    void Fire(const Vector2& direction, float speed);

    class TimerComponent* GetTimer() const { return mTimerComponent; };

    virtual void ManageAnimations() = 0;
    void Kill();
    
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class DrawAnimatedComponent* mDrawAnimatedComponent;
    float mKnockbackIntensity;

    Actor *mShooter;
    float mDieTime;
};
