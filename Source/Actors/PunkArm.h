//
// Created by Pedro Oliveira on 01/07/2025
//

#pragma once

#include "Actor.h"
#include "Punk.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Game.h"
#include "./Projectile.h"
#include "./ProjectileEffect.h"

class PunkArm : public Actor
{
public:
    explicit PunkArm(Game* game, class Punk* punk);

    void OnShoot();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8* keyState) override;
    
    Vector2 mShoulderOffset() {
        return (GetRotation() == 0.0f) ? Vector2(-2.0f, -20.0f) : Vector2(-13.0f, -20.0f);
    }

    Vector2 mShotOffset() {
        return (GetRotation() == 0.0f) ? Vector2(2.0f, -7.0f) : Vector2(-4.0f, -7.0f);
    }

private:
    class DrawSpriteComponent* mDrawComponent;
    class Punk* mPunk;

    Vector2 mTargetPos;
    Vector2 mFireDir;

    friend class Punk;

protected:
    bool mIsShooting;
    float mFireCooldown;
};