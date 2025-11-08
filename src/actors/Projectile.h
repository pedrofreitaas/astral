#pragma once
#include <SDL.h>
#include "../core/Game.h"
#include "Actor.h"
#include "Zoe.h"

class Zoe;
class Game;

class Projectile : public Actor
{
public:
    Projectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    ): Actor(game), mDirection(direction), mSpeed(speed), mKnockbackIntensity(10.f) {
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
        if (mBehaviorState != BehaviorState::Moving) return;

        // projectile can die immediately on collision, so the take damage logic is on the projectile
        if (other->GetLayer() == ColliderLayer::Player)
        {
            auto player = dynamic_cast<Zoe*>(GetGame()->GetZoe());
            if (player)
            {
                Vector2 mSpeedDir = mRigidBodyComponent->GetAppliedForce();
                player->TakeDamage(mSpeedDir * mKnockbackIntensity);
            }
        }
        Kill();
    }

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override {
        if (mBehaviorState != BehaviorState::Moving) return;

        // projectile can die immediately on collision, so the take damage logic is on the projectile
        if (other->GetLayer() == ColliderLayer::Player)
        {
            auto player = dynamic_cast<Zoe*>(GetGame()->GetZoe());
            if (player)
            {
                Vector2 mSpeedDir = mRigidBodyComponent->GetAppliedForce();
                player->TakeDamage(mSpeedDir * mKnockbackIntensity);
            }
        }
        Kill();
    }

    virtual void ManageAnimations() = 0;
    virtual void Kill() { mBehaviorState = BehaviorState::Dying; };
    
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class DrawAnimatedComponent* mDrawAnimatedComponent;
    
    Vector2 mDirection;
    float mKnockbackIntensity;
    float mSpeed;
};
