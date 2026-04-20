#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"

class Quasar : public Enemy
{
public:
    explicit Quasar(Game* game, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:
    bool mAppliedImpulseInAttack;
    Timer* mAttackTimerHandle;
};