//
// Created by Pedro Oliveira on 01/07/2025
//

#pragma once

#include <functional>
#include "Actor.h"
#include "Punk.h"
#include "Weapon.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Game.h"
#include "./Projectile.h"
#include "./ProjectileEffect.h"
#include "../AudioSystem.h"

class PunkArm : public Actor
{
public:
    explicit PunkArm(Game *game, class Punk *punk, const std::function<void()> &onShotCallback);

    void OnShoot();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;

    Vector2 mShoulderOffset();
    Vector2 mShotOffset();

    bool IsAimingRight();
    bool IsAimingLeft();
    bool IsShooting() const { return mIsShooting; }

private:
    class Punk *mPunk;
    class Weapon *mChosenWeapon;
    class Pistol *mPistol;
    class Shotgun *mShotgun;
    Vector2 mTargetPos;
    Vector2 mFireDir;
    float mAngle;
    std::function<void()> mOnShotCallback;

    SoundHandle mDryBulletSoundHandle;
    SoundHandle mShootSoundHandle;

    void ChangeWeapon();

    friend class Punk;

protected:
    bool mIsShooting;
    float mFireCooldown;
};