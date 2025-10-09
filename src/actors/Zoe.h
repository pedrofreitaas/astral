#pragma once
#include "Actor.h"
#include <SDL.h>

const float DEATH_TIMER = 0.71f;

class Zoe : public Actor
{
public:
    explicit Zoe(Game* game, float forwardSpeed = 1500.0f);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    int Lives() { return mLives; }
    void FindHeart();
    
    Vector2 GetCenter() const
    {
        return mColliderComponent->GetCenter();
    }

    void ManageState();

private:
    float mForwardSpeed;
    float mDeathTimer;
    int mLives;
    float mInvincibilityTimer;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    void MaintainInbound();
    void TakeDamage();
    void ManageAnimations();
};