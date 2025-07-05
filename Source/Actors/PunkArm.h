//
// Created by Pedro Oliveira on 01/07/2025
//

#pragma once

#include <functional>
#include <string>
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
    explicit PunkArm(Game *game, class Punk *punk, const std::function<void(Vector2 &recoilForce)> &onShotCallback);

    void OnShoot();

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8 *keyState) override;
    std::string ArmConfig();

    class Punk *GetPunk() const { return mPunk; };
    Vector2 GetTargetPos();
    bool IsAimingRight();
    bool IsAimingLeft();
    bool IsShooting() const { return mIsShooting; }

private:
    class Punk *mPunk;
    class Weapon *mChosenWeapon;
    class Pistol *mPistol; class Shotgun *mShotgun;
    std::function<void(Vector2 &recoilForce)> mOnShotCallback;

    SoundHandle mDryBulletSoundHandle;
    SoundHandle mShootSoundHandle;

    float mChangeWeaponCooldown;
    float mChangeWeaponTimer;

    void ChangeWeapon();

    friend class Punk;

protected:
    bool mIsShooting;
    float mFireCooldown;
};