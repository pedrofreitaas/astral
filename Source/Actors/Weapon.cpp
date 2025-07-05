#include "Weapon.h"

Weapon::Weapon(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : mArm(mArm), mAmmo(maxAmmo), mMaxAmmo(maxAmmo), mFireCooldown(fireCooldown),
      mDrawComponent(nullptr), mReloadCooldown(reloadCooldown)
{
}

void Weapon::Reload(int ammo)
{
    mAmmo = std::min(mAmmo + ammo, mMaxAmmo);
}

bool Weapon::CanShoot()
{
    return mAmmo > 0 && mFireCooldown <= 0.0f;
}

void Weapon::Update(float deltaTime, bool isShooting, bool flip)
{    
    if (mFireCooldown > 0.0f) mFireCooldown -= deltaTime;
    else mFireCooldown = 0.0f;

    if (mReloadCooldown > 0.0f) mReloadCooldown -= deltaTime;
    else {
        mReloadCooldown = 0.0f;
        Reload(1);
    }

    mDrawComponent->SetIsVisible(isShooting);
    mDrawComponent->SetFlip(flip);
}

Pistol::Pistol(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : Weapon(mArm, maxAmmo, fireCooldown, reloadCooldown)
{
    mDrawComponent = new DrawSpriteComponent(mArm, "../Assets/Sprites/Punk/pistol.png", 18, 28, 200);
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));
    mDrawComponent->SetEnabled(false);
}

void Pistol::Shoot(Game *game, Vector2 &start_pos, Vector2 &fire_dir) 
{
    if (!CanShoot())
        return;

    Projectile *projectile = new Projectile(game, ColliderLayer::PlayerProjectile);
    projectile->SetPosition(start_pos);
    projectile->GetComponent<RigidBodyComponent>()->ApplyForce(fire_dir * 3000.0f);

    float angle = atan2f(fire_dir.y, fire_dir.x);
    new ProjectileEffect(game, start_pos, angle);

    mAmmo--;
    mFireCooldown = 0.8f;
}