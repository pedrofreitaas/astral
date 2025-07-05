#include "PunkArm.h"

PunkArm::PunkArm(Game *game, Punk *punk, const std::function<void()> &onShotCallback)
    : Actor(game), mPunk(punk), mIsShooting(false), mFireCooldown(0.0f), mAngle(0.0f)
{
    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Punk/arm_gun.png", 18, 28, 200);
    mDrawComponent->SetPivot(Vector2(0.5f, 0.5f));

    mDrawComponent->SetEnabled(false);

    mOnShotCallback = [this, onShotCallback]() {
        onShotCallback();
    };
}

bool PunkArm::IsAimingRight()
{
    return mTargetPos.x >= mPunk->GetCenter().x && mIsShooting;
}

bool PunkArm::IsAimingLeft()
{
    return mTargetPos.x < mPunk->GetCenter().x && mIsShooting;
}

Vector2 PunkArm::mShoulderOffset()
{
    return (mPunk->GetRotation() == 0.0f) ? Vector2(-2.0f, -20.0f) : Vector2(-13.0f, -20.0f);
}

Vector2 PunkArm::mShotOffset()
{
    return (mPunk->GetRotation() == 0.0f) ? Vector2(2.0f, -7.0f) : Vector2(-4.0f, -7.0f);
}

void PunkArm::OnShoot()
{
    if (mFireCooldown > 0.0f)
        return;

    Projectile *projectile = new Projectile(mGame, ColliderLayer::PlayerProjectile);

    projectile->SetPosition(mPunk->GetCenter() + mShotOffset());
    projectile->GetComponent<RigidBodyComponent>()->ApplyForce(mFireDir * 3000.0f);

    new ProjectileEffect(mGame, mPunk->GetCenter() + mShotOffset(), mAngle);

    mOnShotCallback();
    mFireCooldown = 0.8f;
    mIsShooting = true;
}

void PunkArm::OnProcessInput(const Uint8 *keyState)
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    mTargetPos = Vector2(mouseX,mouseY) + GetGame()->GetCameraPos();
    mFireDir = mTargetPos - mPunk->GetCenter();
    mFireDir.Normalize();
    mAngle = atan2f(mFireDir.y, mFireDir.x);

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
    SetPosition(mPunk->GetCenter() + mShoulderOffset());
    SetRotation(mAngle);
    mDrawComponent->SetFlip(mTargetPos.x <= mPunk->GetCenter().x);
}