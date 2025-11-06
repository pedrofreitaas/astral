#pragma once

#include "../Enemy.h"
#include "../Projectile.h"
#include <SDL.h>

class ZodProjectile : public Projectile
{
public:
    ZodProjectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed
    );
private:
    void ManageAnimations() override;
    void AnimationEndCallback(std::string animationName);
};

class Zod : public Enemy
{
public:
    explicit Zod(Game* game, float forwardSpeed, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;
    void TakeDamage() override;
};