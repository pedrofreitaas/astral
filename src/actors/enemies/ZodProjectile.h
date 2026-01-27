#pragma once

#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include <SDL.h>

class ZodProjectile : public Projectile
{
public:
    ZodProjectile(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed, Actor* zod
    );
private:
    void ManageAnimations() override;
    void OnUpdate(float deltaTime) override;
};