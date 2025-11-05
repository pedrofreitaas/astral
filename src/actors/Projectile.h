#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    ): Actor(game), mDirection(direction), mSpeed(speed) {
        SetPosition(position);
    };

protected:
    void OnUpdate(float deltaTime) override {
        ManageAnimations();

        if (mBehaviorState == BehaviorState::Dying)
            return;
        
        Vector2 movement = mDirection * mSpeed * deltaTime;
        mRigidBodyComponent->ApplyForce(movement);
    }

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override {
        Kill();
    }

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override {
        Kill();
    }

    virtual void ManageAnimations() = 0;
    virtual void Kill() { mBehaviorState = BehaviorState::Dying; };

    class DrawAnimatedComponent* mDrawAnimatedComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    
    Vector2 mDirection;
    float mSpeed;
};
