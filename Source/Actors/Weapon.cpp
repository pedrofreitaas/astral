#include "Weapon.h"

Weapon::Weapon(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : mArm(mArm), mAmmo(maxAmmo), mMaxAmmo(maxAmmo), mFireCooldown(fireCooldown), mFireCooldownTimer(fireCooldown),
      mDrawComponent(nullptr), mReloadCooldown(reloadCooldown), mReloadCooldownTimer(reloadCooldown), mEnabled(false)
{
}

void Weapon::Enable()
{
    mEnabled = true;
    mDrawComponent->SetIsVisible(true);
}

void Weapon::Disable()
{
    mEnabled = false;
    mDrawComponent->SetIsVisible(false);
}

void Weapon::Reload(int ammo)
{
    mAmmo = std::min(mAmmo + ammo, mMaxAmmo);
}

bool Weapon::CanShoot()
{
    return mAmmo > 0 && mFireCooldownTimer <= 0.0f && mEnabled;
}

void Weapon::Update(float deltaTime, bool isShooting, bool flip)
{    
    if (!mEnabled) {
        mDrawComponent->SetIsVisible(false);
        return;
    }
    
    if (mFireCooldownTimer > 0.0f) mFireCooldownTimer -= deltaTime;
    else mFireCooldownTimer = 0.0f;

    if (mReloadCooldownTimer > 0.0f) mReloadCooldownTimer -= deltaTime;
    else {
        mReloadCooldownTimer = mReloadCooldown;
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
    mDrawComponent->SetIsVisible(false);
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
    mFireCooldownTimer = mFireCooldown;
}

Shotgun::Shotgun(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : Weapon(mArm, maxAmmo, fireCooldown, reloadCooldown)
{
    mDrawComponent = new DrawSpriteComponent(mArm, "../Assets/Sprites/Punk/pistol.png", 18, 28, 200);
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));
    mDrawComponent->SetIsVisible(false);
}

void Shotgun::Shoot(Game *game, Vector2 &start_pos, Vector2 &fire_dir) 
{
    if (!CanShoot())
        return;

    for (int i = -1; i <= 1; ++i) {
        Projectile *projectile = new Projectile(game, ColliderLayer::PlayerProjectile);
        projectile->SetPosition(start_pos);
        Vector2 spreadDir = fire_dir + Vector2(0.1f * i, 0.0f); // Spread the shot
        spreadDir.Normalize();
        projectile->GetComponent<RigidBodyComponent>()->ApplyForce(spreadDir * 3000.0f);
    }

    // float angle = atan2f(fire_dir.y, fire_dir.x);
    // new ProjectileEffect(game, start_pos, angle);

    mAmmo--;
    mFireCooldownTimer = mFireCooldown;
}