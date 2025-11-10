#pragma once

#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include <SDL.h>

class SithProjectile : public Projectile
{
public:
    SithProjectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    );

private:
    void ManageAnimations() override;
    void AnimationEndCallback(std::string animationName);
};

class Sith : public Enemy
{
public:
    enum class Attacks {
        None,
        Attack1,
        Attack2
    };

    float PROJECTICLE_COOLDOWN = 30.f;
    float PROJECTILE_SPEED = 50000.f;

    float ATTACK1_COOLDOWN = 8.f;
    float ATTACK2_COOLDOWN = 20.f;
    float ATTACK2_EXTRA_SPEED = 25000.f;

    explicit Sith(Game* game, float forwardSpeed, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;

    void FireProjectile();
    void SetProjectileOnCooldown(bool onCooldown) { mIsProjectileOnCooldown = onCooldown; }

    void Attack1();
    void SetAttack1OnCooldown(bool onCooldown) { mIsAttack1OnCooldown = onCooldown; }

    void Attack2();
    void SetAttack2OnCooldown(bool onCooldown) { mIsAttack2OnCooldown = onCooldown; }

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    Vector2 GetProjectileOffset() { 
        if (GetRotation() == 0.f) {
            return Vector2(32.f,30.f);
        }
        else {
            return Vector2(22.f,30.f);
        }
    }

private:
    bool mIsProjectileOnCooldown, mIsAttack1OnCooldown, mIsAttack2OnCooldown;
    Attacks mCurrentAttack;
    TimerComponent *mTimerComponent;
};