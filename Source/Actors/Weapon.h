//
// Created by Pedro Oliveira on 04/07/2025
//

#pragma once

#include "Actor.h"
#include "Punk.h"
#include "PunkArm.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Game.h"
#include "./Projectile.h"
#include "./ProjectileEffect.h"

class Weapon {
public:
    Weapon(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown);

    virtual void Shoot(Game *game, Vector2 &start_pos, Vector2 &fire_dir) = 0;

    void Reload(int ammo);
    bool CanShoot();
    void Update(float deltaTime, bool isShooting, bool flip);

    class PunkArm *mArm;
    int mAmmo;
    int mMaxAmmo;
    float mFireCooldown;
    float mReloadCooldown;
    DrawSpriteComponent *mDrawComponent;
};

class Pistol : public Weapon
{
public:
    Pistol(class PunkArm *mArm, int maxAmmo = 5, float fireCooldown = 0.8f, float reloadCooldown = 1.5f);
    void Shoot(Game *game, Vector2 &start_pos, Vector2 &fire_dir) override;
};