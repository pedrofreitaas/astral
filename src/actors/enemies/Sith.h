#pragma once

#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include <SDL.h>

class SithProjectile : public Projectile
{
public:
    SithProjectile(
        class Game* game, const std::string &spriteSheetPath, 
        const std::string &spriteSheetData, Vector2 direction, 
        Vector2 position, float speed
    );

private:
    void ManageState() override;
    void ManageAnimations() override;
    void Kill() override;
    void AnimationEndCallback(std::string animationName);
};

class Sith : public Enemy
{
public:
    explicit Sith(Game* game, float forwardSpeed, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;
    void TakeDamage() override;

    void FireProjectile(Vector2 &direction, float speed);

    void OnUpdate(float deltaTime) override;

private:
    float mAttackCooldown;
    float mAttackCooldownTimer;

    TimerComponent *mTimerComponent;
};