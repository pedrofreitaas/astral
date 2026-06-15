#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"

class Rock : public Projectile
{
public:
    static void SpawnRocks(Game *game, class Zathura* zathura);

    Rock(
        class Game* game, Vector2 position, Actor* zathura
    );
    
    void ThrowRock(const Vector2& target);

private:
    void ManageAnimations() override;
    void OnUpdate(float deltaTime) override;

    TimerComponent* mTimerComponent;
};