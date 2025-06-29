#pragma once
#include "Actor.h"

class ProjectileEffect : public Actor
{
public:
    ProjectileEffect(class Game* game, const Vector2& position, float rotation = 0.0f);

    void OnUpdate(float deltaTime) override;

private:
    float mDeathTimer;

    class DrawSpriteComponent* mDrawComponent;
};