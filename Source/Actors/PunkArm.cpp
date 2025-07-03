#include "PunkArm.h"

PunkArm::PunkArm(Game *game, Punk *punk)
    : Actor(game), mPunk(punk), mIsShooting(false), mFireCooldown(0.0f)
{
    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Punk/arm_gun.png", 18, 28, 200);
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));

    mDrawComponent->SetEnabled(false);
}

void PunkArm::OnShoot()
{
    if (mFireCooldown > 0.0f)
        return;

    float angle = atan2f(mFireDir.y, mFireDir.x);
    
    mDrawComponent->SetFlip(mTargetPos.x <= mPunk->GetCenter().x);

    Projectile *projectile = new Projectile(mGame, ColliderLayer::PlayerProjectile);

    projectile->SetPosition(mPunk->GetCenter() + mShotOffset());
    projectile->GetComponent<RigidBodyComponent>()->ApplyForce(mFireDir * 3000.0f);

    new ProjectileEffect(mGame, mPunk->GetCenter() + mShotOffset(), angle);

    mFireCooldown = 0.8f;
}

void PunkArm::OnProcessInput(const Uint8 *keyState)
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    mTargetPos = Vector2(mouseX,mouseY) + GetGame()->GetCameraPos();
    mFireDir = mTargetPos - mPunk->GetCenter();
    mFireDir.Normalize();

    if (!mouseState || !SDL_BUTTON(SDL_BUTTON_LEFT)) {
        mIsShooting = false;
        return;
    }

    OnShoot();
}

void PunkArm::OnUpdate(float deltaTime)
{
    if (mFireCooldown >= 0.0f) mFireCooldown -= deltaTime;
    else mFireCooldown = 0.0f;

    mDrawComponent->SetIsVisible(mIsShooting);

    float angle = atan2f(mFireDir.y, mFireDir.x);
    SetRotation(angle);
}