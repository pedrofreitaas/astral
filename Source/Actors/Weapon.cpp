#include "Weapon.h"

Weapon::Weapon(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : mArm(mArm), mAmmo(maxAmmo), mMaxAmmo(maxAmmo), 
      mFireCooldown(fireCooldown), mFireCooldownTimer(fireCooldown),
      mDrawComponent(nullptr), mReloadCooldown(reloadCooldown), 
      mReloadCooldownTimer(reloadCooldown), mEnabled(false),
      mPunkArmConfig("shooting_left_arm")
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

    if (mReloadCooldownTimer > 0.0f && !mArm->IsShooting()) mReloadCooldownTimer -= deltaTime;
    
    if (mReloadCooldownTimer <= 0.0f) {
        mReloadCooldownTimer = mReloadCooldown;
        Reload(1);
    }

    mDrawComponent->SetIsVisible(isShooting);
    mDrawComponent->SetFlip(flip);
}

Pistol::Pistol(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : Weapon(mArm, maxAmmo, fireCooldown, reloadCooldown)
{
    mDrawComponent = new DrawSpriteComponent(
        mArm, 
        "../Assets/Sprites/Punk/pistol.png", 
        48, 48, 
        static_cast<int>(DrawLayerPosition::Player) - 10
    );
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));
    mDrawComponent->SetIsVisible(false);
}

Vector2 Pistol::ShotOffset() 
{
    return Vector2(0.0f, 0.0f);
}

Vector2 Pistol::Shoot(Vector2 &target) 
{
    if (!CanShoot())
        return Vector2::Zero;

    Projectile *projectile = new Projectile(
        mArm->GetPunk()->GetGame(),
        ColliderLayer::PlayerProjectile
    );

    Vector2 punkCenter = mArm->GetPunk()->GetCenter();
    Vector2 startPos = punkCenter + ShotOffset();

    Vector2 fireDir = target - startPos;
    fireDir.Normalize();

    projectile->SetPosition(startPos);
    projectile->mPreviousPosition = startPos;
    projectile->GetComponent<RigidBodyComponent>()->ApplyForce(fireDir * 3000.0f);

    float angle = atan2f(fireDir.y, fireDir.x);
    new ProjectileEffect(mArm->GetPunk()->GetGame(), startPos, angle);

    mAmmo--;
    mFireCooldownTimer = mFireCooldown;

    return fireDir * 3000.0f;
}

Shotgun::Shotgun(class PunkArm *mArm, int maxAmmo, float fireCooldown, float reloadCooldown)
    : Weapon(mArm, maxAmmo, fireCooldown, reloadCooldown)
{
    mDrawComponent = new DrawSpriteComponent(
        mArm, 
        "../Assets/Sprites/Punk/shotgun.png", 
        48, 48, 
        static_cast<int>(DrawLayerPosition::Sky)
    );
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));
    mDrawComponent->SetIsVisible(false);
    mPunkArmConfig = "shooting_noarm";
}

Vector2 Shotgun::ShotOffset() 
{
    return Vector2(0.0f, 0.0f);
}

Vector2 Shotgun::Shoot(Vector2 &target) 
{
    if (!CanShoot())
        return Vector2::Zero;

    Vector2 punkCenter = mArm->GetPunk()->GetCenter();
    Vector2 startPos = punkCenter + ShotOffset();

    Vector2 fireDir = target - startPos;
    fireDir.Normalize();

    for (int i = -1; i <= 1; ++i) {
        Projectile *projectile = new Projectile(mArm->GetPunk()->GetGame(), ColliderLayer::PlayerProjectile);
        projectile->SetPosition(startPos);
        projectile->mPreviousPosition = startPos;
        projectile->GetComponent<RigidBodyComponent>()->ApplyForce(fireDir * Matrix2::CreateRotation(i * Math::Pi/6.0f) * 1800.0f);
    }

    mAmmo--;
    mFireCooldownTimer = mFireCooldown;

    return fireDir * 12000.f;
}