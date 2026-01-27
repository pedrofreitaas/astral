#pragma once

#include "../Enemy.h"
#include "../Projectile.h"
#include "../../components/TimerComponent.h"
#include <SDL.h>

class Zod : public Enemy
{
public:
    explicit Zod(Game* game, float forwardSpeed, const Vector2& position);

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    
    Vector2 GetProjectileOffset() { 
        if (GetRotation() == 0.f) {
            return Vector2(37.f,8.f);
        }
        else {
            return Vector2(9.f,8.f);
        }
    }

private:
    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;

    void FireProjectile();

    bool mProjectileOnCooldown;
    class TimerComponent *mTimerComponent;
};