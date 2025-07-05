#include "PunkArm.h"

PunkArm::PunkArm(Game *game, Punk *punk, const std::function<void()> &onShotCallback)
    : Actor(game), mPunk(punk), mIsShooting(false), mFireCooldown(0.0f), mAngle(0.0f),
      mTargetPos(Vector2::Zero), mFireDir(Vector2::Zero), mPistol(nullptr)
{
    mPistol = new Pistol(this);

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
    if (!mPistol->CanShoot()) return;

    Vector2 shotPos = mPunk->GetCenter() + mShotOffset();
    mPistol->Shoot(GetGame(), shotPos, mFireDir);
    mOnShotCallback();
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

    SetPosition(mPunk->GetCenter() + mShoulderOffset());
    SetRotation(mAngle);

    mPistol->Update(deltaTime, mIsShooting, mTargetPos.x <= mPunk->GetCenter().x);
}