#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include "../Collider.h"

class SithProjectile : public Projectile
{
public:
    SithProjectile(
        class Game* game, Vector2 position, 
        Vector2 direction, Actor* sith
    );

private:
    void ManageAnimations() override;
    void AnimationEndCallback(std::string animationName);
};