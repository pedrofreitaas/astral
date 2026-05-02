#include "./Zoe.h"

void Zoe::OnJumpPressed()
{
    if (!IsSDLButtonBlocked(Zoe::JUMP_BUTTON))
    {
        mIsTryingToJump = true;
    }
}

void Zoe::OnJumpReleased()
{
    mIsTryingToJump = false;
}

void Zoe::OnAttackPressed()
{
    if (!IsSDLButtonBlocked(Zoe::HIT_BUTTON))
    {
        mIsTryingToHit = true;
    }
}

void Zoe::OnAttackReleased()
{
    mIsTryingToHit = false;
}

void Zoe::OnDodgePressed()
{
    if (!IsSDLButtonBlocked(Zoe::DODGE_BUTTON))
    {
        mIsTryingToDodge = true;
    }
}

void Zoe::OnDodgeReleased()
{
    mIsTryingToDodge = false;
}

void Zoe::OnVentaniaPressed()
{
    if (!IsSDLButtonBlocked(Zoe::VENTANIA_BUTTON))
    {
        mTryingToTriggerVentania = true;
    }
}

void Zoe::OnVentaniaReleased()
{
    mTryingToTriggerVentania = false;
}

void Zoe::OnFireballPressed()
{
    if (!IsSDLButtonBlocked(Zoe::FIREBALL_BUTTON))
    {
        mTryingToFireFireball = true;
    }
}

void Zoe::OnFireballReleased()
{
    mTryingToFireFireball = false;
}

void Zoe::OnNevascaPressed()
{
    if (!IsSDLButtonBlocked(Zoe::NEVASCA_BUTTON))
    {
        mIsTryingToNevasca = true;
    }
}

void Zoe::OnNevascaReleased()
{
    mIsTryingToNevasca = false;
}

void Zoe::CheckAbilitiesKeys(const std::vector<SDL_Event>& events, SDL_GameController* controller)
{
    // these abilities should be reset every frame - to avoid keeping them pressed for trigger
    mIsTryingToJump = false;
    mIsTryingToHit = false;
    mIsTryingToDodge = false;
    // mTryingToTriggerVentania = false; - this should be per frame, but there is BUG - TODO
    // mTryingToFireFireball = false; - this must be because there is a charging part.
    // mIsTryingToNevasca = false; - isnt per frame!

    if (mAbilitiesLocked || controller == nullptr) return;

    for (const auto &event : events)
    {
        switch (event.type)
        {
        case SDL_CONTROLLERBUTTONDOWN:
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
            {
                OnJumpPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
            {
                OnAttackPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
            {
                OnFireballPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
            {
                OnDodgePressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
            {
                OnVentaniaPressed();
            }

            break;

        case SDL_CONTROLLERBUTTONUP:
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
            {
                OnJumpReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
            {
                OnAttackReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
            {
                OnFireballReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
            {
                OnDodgeReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
            {
                OnVentaniaReleased();
            }

            break;
        }
    }

    if (
        !IsSDLButtonBlocked(Zoe::NEVASCA_BUTTON) && 
        SDL_GameControllerGetAxis(controller, Zoe::NEVASCA_AXIS) > 0
    )
    {
        OnNevascaPressed();
    }
    else
    {
        OnNevascaReleased();
    }
}

static Vector2 SnapVentaniaDir(Vector2 dir)
{
    // Right, Right-Up, Up, Left-Up, Left
    static const Vector2 DIRS[] = {
        Vector2(1.f, 0.f),
        Vector2(0.7071f, -0.7071f),
        Vector2(0.f, -1.f),
        Vector2(-0.7071f, -0.7071f),
        Vector2(-1.f, 0.f),
    };

    float bestDot = -2.f;
    Vector2 best = DIRS[0];
    for (const Vector2 &d : DIRS)
    {
        float dot = dir.x * d.x + dir.y * d.y;
        if (dot > bestDot)
        {
            bestDot = dot;
            best = d;
        }
    }
    return best;
}

void Zoe::TriggerVentania()
{
    if (!mLandedAfterVentania)
        return;

    if (!mTryingToTriggerVentania)
        return;

    Vector2 rawDir = mInputMovementDir.LengthSq() > 0.f ? mInputMovementDir : GetForward();
    Vector2 dir = SnapVentaniaDir(rawDir);

    // distance param has to be bigger than IsPressingAgainstWall!
    int closeToWall = mColliderComponent->IsCloseToTileWallHorizontally(3.f);

    if (
        closeToWall != 0 && dir.x == 0 && dir.y < 0 ||
        (Math::Sign(closeToWall) == Math::Sign(dir.x)))
    {
        return;
    }

    float speed = mGame->GetConfig()->Get<float>("ZOE.POWERS.VENTANIA.SPEED");

    mRigidBodyComponent->ResetVelocity();
    mRigidBodyComponent->ApplyImpulse(dir * speed);

    new Ventania(
        GetGame(),
        GetCenter(),
        dir);

    SetLandedAfterVentania(false);

    mGame->GetAudio()->PlaySound("ventania.wav");

    SetBehaviorState(BehaviorState::Dashing);

    mDashGravityDisableTimer->Restart();
}

bool Zoe::CheckJump()
{
    if (!mGame->GetApplyGravityScene())
        return false;

    if (!mIsTryingToJump)
        return false;

    if (mBehaviorState == BehaviorState::Jumping)
        return false;

    if (mBehaviorState == BehaviorState::Clinging)
        return false;

    float coyoteTimeRemaining = mTimerComponent->checkTimerRemaining(mCoyoteTimer);

    bool coyoteExpired = coyoteTimeRemaining <= 0.f;

    if (mBehaviorState == BehaviorState::Falling && coyoteExpired)
        return false;

    float jumpForce = mRigidBodyComponent->GetJumpImpulseY(3);

    mRigidBodyComponent->ApplyImpulse(
        Vector2(0.f, jumpForce));

    SetBehaviorState(BehaviorState::Jumping);

    return true;
}

bool Zoe::CheckDodge()
{
    if (!mIsTryingToDodge || mBehaviorState == BehaviorState::Dodging)
        return false;

    if (mDodgeCooldownTimer != nullptr && mTimerComponent->checkTimerRemaining(mDodgeCooldownTimer) > 0.f)
    {
        return false;
    }

    SetBehaviorState(BehaviorState::Dodging);
    mColliderComponent->SetIgnoreLayers(Zoe::IGNORED_LAYERS_DODGE);
    mColliderComponent->SetBB(&DODGE_BB);

    return true;
}

void Zoe::DodgeEnd()
{
    if (mBehaviorState != BehaviorState::Dodging)
        return;

    SetBehaviorState(BehaviorState::Idle);

    Vector2 currentPos = GetPosition();
    SetPosition(Vector2(currentPos.x, currentPos.y - 5));

    mColliderComponent->SetIgnoreLayers(Zoe::IGNORED_LAYERS_DEFAULT);
    mColliderComponent->SetBB(&DEFAULT_BB);

    float cooldown = mGame->GetConfig()->Get<float>("ZOE.DODGE_COOLDOWN");
    mDodgeCooldownTimer = mTimerComponent->AddTimer(cooldown, nullptr);
}

void Zoe::FireFireball()
{
    if (CheckFireballOnCooldown())
        return;

    float manaCost = mGame->GetConfig()->Get<float>("ZOE.POWERS.FIREBALL.MANA_COST");

    if (!HasMana(manaCost))
        return;

    ConsumeMana(manaCost);

    new Fireball(
        mGame,
        GetPosition() + GetFireballOffset(),
        GetForward(),
        this);

    float cooldown = mGame->GetConfig()->Get<float>("ZOE.POWERS.FIREBALL.COOLDOWN");
    mFireballCooldownTimer = mTimerComponent->AddTimer(cooldown, [this]
                                                       { mFireballCooldownTimer = nullptr; });
    mGame->GetAudio()->PlaySound("fireball.wav");
}

bool Zoe::CheckNevasca()
{
    if (!mIsTryingToNevasca)
    {
        return false;
    }

    if (mBehaviorState != BehaviorState::Idle)
    {
        return false;
    }

    mIsFiringNevasca = true;

    Vector2 nevascaDir = mInputMovementDir;

    if (nevascaDir.LengthSq() == 0.f)
    {
        nevascaDir = GetForward();
    }

    float ratePerSecond = mGame->GetConfig()->Get<int>("ZOE.POWERS.NEVASCA.RATE_PER_SECOND");

    float rate = 1.f / ratePerSecond;

    float dissipationAngle = 15.f;
    std::vector<Vector2> dirs = {
        nevascaDir,
        Vector2::RotateVec(nevascaDir, dissipationAngle),
        Vector2::RotateVec(nevascaDir, -dissipationAngle)};

    while (mNevascaTimer >= rate)
    {
        if (HasMana(mGame->GetConfig()->Get<float>("ZOE.POWERS.NEVASCA.MANA_COST")))
        {
            ConsumeMana(mGame->GetConfig()->Get<float>("ZOE.POWERS.NEVASCA.MANA_COST"));
        }
        else
        {
            return false;
        }

        mNevascaTimer -= rate;

        for (const Vector2 &dir : dirs)
        {
            new Nevasca(
                mGame,
                GetNevascaOffset(),
                dir,
                this);
        }
    }

    if (!mNevascaSoundHandle.IsValid() ||
        mGame->GetAudio()->GetSoundState(mNevascaSoundHandle) == SoundState::Stopped)
    {
        mNevascaSoundHandle = mGame->GetAudio()->PlaySound("nevasca.wav");
    }

    Vector2 pads = mGame->getNormalizedControlerPad();

    if (pads.x > 0.f)
    {
        SetRotation(0.f);
    }
    else if (pads.x < 0.f)
    {
        SetRotation(Math::Pi);
    }

    SetBehaviorState(BehaviorState::Idle);

    return true;
}

bool Zoe::CheckHit()
{
    if (!mIsTryingToHit)
        return false;
    if (mBehaviorState == BehaviorState::AerialAttacking)
        return false;
    if (mBehaviorState == BehaviorState::Attacking)
        return false;

    mGame->SetCameraCenterToShake(0.25f);

    bool onGround = mRigidBodyComponent->GetOnGround();

    SetBehaviorState(onGround ? BehaviorState::Attacking : BehaviorState::AerialAttacking);
    mGame->GetAudio()->PlaySound("zoeSmash.wav");

    if (onGround)
    {
        mGame->SetCameraCenterToShake(0.25f);

        mAttackCollider = new Collider(
            mGame,
            this,
            GetCenter() + (GetRotation() == 0.f ? Vector2(11, -16) : Vector2(-25, -16)),
            Vector2(14, 24),
            nullptr,
            DismissOn::Both,
            ColliderLayer::PlayerAttack,
            {ColliderLayer::Player},
            .75f,
            nullptr,
            false,
            std::bind(&Zoe::GetCenter, this));

        return true;
    }

    float ySpeed = mRigidBodyComponent->GetJumpImpulseY(.5f);
    mRigidBodyComponent->ApplyImpulse(Vector2(0.f, ySpeed));

    // this collider moves with player in manageState
    mAerialAttackCollider = new Collider(
        mGame,
        this,
        GetCenter() - Vector2(20, 30),
        Vector2(40, 40),
        nullptr,
        DismissOn::Time,
        ColliderLayer::PlayerAttack,
        {ColliderLayer::Player},
        1.25f,
        nullptr,
        false,
        std::bind(&Zoe::GetCenter, this));

    return true;
}
