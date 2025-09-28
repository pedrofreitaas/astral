//
// Created by Pedro Oliveira on 04/07/2025
//

#pragma once

#include <string>
#include "Actor.h"
#include "Zoe.h"
#include "PunkArm.h"
#include "../components/draw/DrawSpriteComponent.h"
#include "../core/Game.h"
#include "./Projectile.h"
#include "../libs/Math.h"

class Weapon {
public:
    Weapon(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown);

    // fires the bullet and returns the direction of the shot
    virtual Vector2 Shoot(Vector2 &target) = 0;

    void Disable(); void Enable();
    void Reload(int ammo);
    bool CanShoot();
    void Update(float deltaTime, bool isShooting, bool flip);
    virtual Vector2 ShotOffset() = 0;

    bool mEnabled;
    class PunkArm *mArm;
    int mAmmo; int mMaxAmmo;
    const float mFireCooldown; float mFireCooldownTimer;
    const float mReloadCooldown; float mReloadCooldownTimer;
    DrawSpriteComponent *mDrawComponent;
    std::string mPunkArmConfig;
};

class Pistol : public Weapon
{
public:
    Pistol(class PunkArm *mArm, int maxAmmo = 5, float fireCooldown = 0.8f, float reloadCooldown = 1.5f);
    Vector2 ShotOffset() override;
    Vector2 Shoot(Vector2 &target) override;
};

class Shotgun : public Weapon
{
public:
    Shotgun(class PunkArm *mArm, int maxAmmo = 2, float fireCooldown = 1.4f, float reloadCooldown = 4.0f);
    Vector2 ShotOffset() override;
    Vector2 Shoot(Vector2 &target) override;
};