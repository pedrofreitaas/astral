#pragma once
#include <SDL.h>
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/draw/DrawAnimatedComponent.h"

class Nevasca : public Projectile
{
public:
    Nevasca(
        class Game* game, Vector2 position, 
        Vector2 direction, Actor* shooter
    );

private:
    void ManageAnimations() {};

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void OnUpdate(float deltaTime) override;

    void Kill() override;
};

