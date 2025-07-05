#include "PunkArm.h"

PunkArm::PunkArm(Game *game, Punk *punk, const std::function<void(Vector2 &recoilForce)> &onShotCallback)
    : Actor(game), mPunk(punk), mIsShooting(false),
      mPistol(nullptr), mShotgun(nullptr), mChosenWeapon(nullptr),
      mChangeWeaponCooldown(.5f), mChangeWeaponTimer(.5f)
{
    mPistol = new Pistol(this);
    mShotgun = new Shotgun(this);
    
    mChosenWeapon = mPistol;
    mChosenWeapon->Enable();

    mOnShotCallback = [this, onShotCallback](Vector2 &recoilForce) {
        onShotCallback(recoilForce);
    };
}

std::string PunkArm::ArmConfig()
{
    if (!mChosenWeapon) return "idle";
    return mChosenWeapon->mPunkArmConfig;
}

Vector2 PunkArm::GetTargetPos()
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    
    Vector2 target = Vector2(mouseX, mouseY);
    return target + GetGame()->GetCameraPos();
}

bool PunkArm::IsAimingRight()
{
    return GetTargetPos().x >= mPunk->GetCenter().x && mIsShooting;
}

bool PunkArm::IsAimingLeft()
{
    return GetTargetPos().x < mPunk->GetCenter().x && mIsShooting;
}

void PunkArm::OnShoot()
{
    if (!mChosenWeapon->CanShoot()) {
        if (mGame->GetAudio()->GetSoundState(mDryBulletSoundHandle) != SoundState::Playing && 
            mChosenWeapon->mAmmo <= 0)
        {
            mDryBulletSoundHandle = mGame->GetAudio()->PlaySound("DryFire.ogg");
        }
        return;
    };

    Vector2 target = GetTargetPos();
    Vector2 recoilDir = mChosenWeapon->Shoot(target) *-1;
    
    mOnShotCallback(recoilDir);
    
    mIsShooting = true;

    if (mGame->GetAudio()->GetSoundState(mShootSoundHandle) == SoundState::Playing) {
        mGame->GetAudio()->StopSound(mShootSoundHandle);
    }
    mShootSoundHandle = mGame->GetAudio()->PlaySound("Fire.wav");
}

void PunkArm::OnProcessInput(const Uint8 *keyState)
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (!mouseState || !SDL_BUTTON(SDL_BUTTON_LEFT)) {
        mIsShooting = false;
        return;
    }

    OnShoot();
}

void PunkArm::OnUpdate(float deltaTime)
{   
    if (mChangeWeaponTimer > 0.0f) mChangeWeaponTimer -= deltaTime;
    
    Vector2 fireDir = GetTargetPos() - GetPunk()->GetCenter();
    fireDir.Normalize();
    float angle = atan2f(fireDir.y, fireDir.x);
    
    SetRotation(angle);
    SetPosition(mPunk->GetPosition());
    mChosenWeapon->Update(deltaTime, mIsShooting, GetTargetPos().x <= mPunk->GetCenter().x);
}

void PunkArm::ChangeWeapon()
{
    if (mChangeWeaponTimer > 0.0f) return;

    if (mChosenWeapon == mPistol && mShotgun != nullptr) {
        mChosenWeapon = mShotgun;
        mPistol->Disable();
    } 
    
    else if (mChosenWeapon == mShotgun && mPistol != nullptr) {
        mChosenWeapon = mPistol;
        mShotgun->Disable();
    }

    mChosenWeapon->Enable();
    mChangeWeaponTimer = mChangeWeaponCooldown;
    // mGame->GetAudio()->PlaySound("ChangeWeapon.wav");
}