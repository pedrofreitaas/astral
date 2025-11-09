#pragma once
#include "Actor.h"
#include "Projectile.h"
#include <SDL.h>

const float DEATH_TIMER = 0.71f;

class Ventania : public Actor
{
public:
    Ventania(Game* game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed = .0f);

private:    
    class DrawAnimatedComponent *mDrawAnimatedComponent;
    void AnimationEndCallback(std::string animationName);
};

class Fireball : public Projectile
{
    int MAX_RICOCHETS = 3;

public:
    Fireball(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    );

private:
    void ManageAnimations();
    void AnimationEndCallback(std::string animationName);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    int mRicochetsCount;
};

class Zoe : public Actor
{
    float FIREBALL_SPEED = 20000.f;
    float FIREBALL_COOLDOWN = 4.f;
    
    float VETANIA_SPEED = 17000.f;
    float VETANIA_COOLDOWN = 0.75f;

public:
    explicit Zoe(Game* game, float forwardSpeed = 2000.0f);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    void ManageState();
    void SetInvincible(bool invincible) { mInvincible = invincible; }
    void AnimationEndCallback(std::string animationName);
    
    Vector2 GetFireballOffset() {
        if (GetRotation() == 0.f) {
            return Vector2(48.f,34.f);
        }
        else {
            return Vector2(14.f,34.f);
        }
    }

private:
    float mForwardSpeed;
    float mDeathTimer;

    class TimerComponent *mTimerComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    void ManageAnimations();

    void FireFireball();
    void SetFireballOnCooldown(bool onCooldown) { mIsFireballOnCooldown = onCooldown; }
    bool mIsFireballOnCooldown, mTryingToFireFireball;

    void TriggerVentania();
    void SetVentaniaOnCooldown(bool onCooldown) { mIsVentaniaOnCooldown = onCooldown; }
    bool mIsVentaniaOnCooldown, mTryingToTriggerVentania;
};
