#pragma once

#include "../Enemy.h"
#include <SDL.h>

class Zod : public Enemy
{
public:
    explicit Zod(Game* game, float forwardSpeed, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;
    void TakeDamage() override;
};