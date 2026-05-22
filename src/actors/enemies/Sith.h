#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include "../Collider.h"
#include "./SithProjectile.h"

class Sith : public Enemy
{
public:
    enum class Attacks {
        None,
        Attack1,
        Attack2
    };

    explicit Sith(Game* game, const Vector2& position);

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

private:
    bool mIsProjectileOnCooldown, mIsAttack1OnCooldown, mIsAttack2OnCooldown;
    Attacks mCurrentAttack;
    TimerComponent *mTimerComponent;

    Collider *mAttack2Collider;
    bool mHasAppliedAttackBoost;
};