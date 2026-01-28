#pragma once
#include <SDL.h>
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "../components/collider/AABBColliderComponent.h"

class Fireball : public Projectile
{
    int MAX_RICOCHETS = 3;

public:
    Fireball(
        class Game* game, Vector2 position, 
        Vector2 direction, Actor* shooter
    );

private:
    void ManageAnimations();
    void AnimationEndCallback(std::string animationName);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;

    int mRicochetsCount;
};
