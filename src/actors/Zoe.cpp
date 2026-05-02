#include "Zoe.h"

Zoe::Zoe(
    Game *game, const float forwardSpeed, const Vector2 &center)
    : Actor(game, game->GetConfig()->Get<int>("ZOE.LIFE_POINTS"), true, "zoe"), mForwardSpeed(forwardSpeed),
      mTryingToFireFireball(false), mFireballCooldownTimer(), mDodgeCooldownTimer(),
      mLandedAfterVentania(false), mTryingToTriggerVentania(false),
      mIsTryingToHit(false), mAttackCollider(nullptr), mIsTryingToDodge(false),
      mInputMovementDir(0.f, 0.f), mMovementLocked(false), mAbilitiesLocked(false),
      mDamageSoundHandle(SoundHandle::Invalid), mIsTryingToJump(false), mNevascaSoundHandle(SoundHandle::Invalid),
      mIsTryingToNevasca(false), mIsFiringNevasca(false), mNevascaTimer(0.f), mAerialAttackCollider(nullptr),
      mCoyoteTimer(nullptr), mDashGravityDisableTimer(nullptr), mCurrentCheckpoint(nullptr), mDeaths(0), 
      mMana(game->GetConfig()->Get<float>("ZOE.MAX_MANA"))
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 11.0f);

    mColliderComponent = new AABBColliderComponent(
        this,
        DEFAULT_BB.x, DEFAULT_BB.y, DEFAULT_BB.w, DEFAULT_BB.h,
        ColliderLayer::Player);

    mTimerComponent = new TimerComponent(this);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Zoe/texture.png",
        "../assets/Sprites/Zoe/texture.json", // aseprite format
        std::bind(&Zoe::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("idle", 0, 8);
    mDrawComponent->AddAnimation("ground-crush", 9, 14);
    mDrawComponent->AddAnimation("blink", 15, 19);
    mDrawComponent->AddAnimation("jump", {20, 21});
    mDrawComponent->AddAnimation("run", 22, 25);
    mDrawComponent->AddAnimation("hurt", 26, 29);
    mDrawComponent->AddAnimation("dodging", {30});
    mDrawComponent->AddAnimation("aerial-crush", 31, 40);
    mDrawComponent->AddAnimation("clinging", 41, 43);
    mDrawComponent->AddAnimation("charging", 44, 49);
    mDrawComponent->AddAnimation("spraying", 47, 49);

    mDrawComponent->SetAnimation("idle");

    mColliderComponent->SetIgnoreLayers(
        Zoe::IGNORED_LAYERS_DEFAULT);

    SetPosition(center - GetHalfSize());

    mGame->SetZoe(this);

    mCoyoteTimer = mTimerComponent->AddNotRemovableTimer(0.1f, nullptr);

    // making this timer longer results in a buggy dash.
    mDashGravityDisableTimer = mTimerComponent->AddNotRemovableTimer(0.1f, nullptr);

    SetOnDamageCallback(std::bind(&Zoe::OnDamageCallback, this));

    mManaRegenTimerHandle = mTimerComponent->AddNotRemovableTimer(
        1.f, 
        [this]() {
            RegenerateMana();
            mManaRegenTimerHandle->Restart();
        }
    );
}

Zoe::~Zoe()
{
    mGame->SetZoe(nullptr);
}

void Zoe::OnProcessInput(const uint8_t *state, const std::vector<SDL_Event> &events)
{
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing)
    {
        mIsTryingToDodge = false;
        mIsTryingToJump = false;
        mIsTryingToHit = false;
        mTryingToFireFireball = false;
        mTryingToTriggerVentania = false;
        mInputMovementDir = Vector2(0.f, 0.f);
        return;
    }

    SDL_GameController *controller = GetGame()->GetController();

    CheckAbilitiesKeys(events, controller);

    if (mMovementLocked)
    {
        mInputMovementDir = Vector2(0.f, 0.f);
        return;
    }

    Vector2 pads = mGame->getNormalizedControlerPad();

    mInputMovementDir = Vector2(pads.x, pads.y);

    float lengthSq = mInputMovementDir.LengthSq();
    if (lengthSq > 0.0f)
    {
        mInputMovementDir *= 1.0f / Math::Sqrt(lengthSq);
    }
}

void Zoe::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mBehaviorState == BehaviorState::Dying)
        return;
}

void Zoe::ManageState()
{
    bool isCutscene = mGame->GetGamePlayState() == Game::GamePlayState::PlayingCutscene;

    if (
        mPreviousBehaviorState == BehaviorState::Dashing &&
        !mRigidBodyComponent->GetApplyGravity())
    {
        mRigidBodyComponent->SetApplyGravity(true);
    }

    switch (mBehaviorState)
    {
    case BehaviorState::Dying:
        Kill();
        break;

    case BehaviorState::Jumping:
        if (mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Idle);
            break;
        }

        if (IsPressingAgainstWall() != 0)
        {
            mRigidBodyComponent->ResetVelocityY();
            SetBehaviorState(BehaviorState::Clinging);
            break;
        }

        if (mRigidBodyComponent->GetVelocity().y > 0.f)
        {
            SetBehaviorState(BehaviorState::Falling);
            break;
        }

        if (mTryingToTriggerVentania && mLandedAfterVentania)
        {
            TriggerVentania();
            break;
        }

        if (CheckHit())
            break;

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        Move(1.3f);

        break;

    case BehaviorState::Falling:
        if (mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Idle);
            break;
        }

        if (IsPressingAgainstWall() != 0)
        {
            mRigidBodyComponent->ResetVelocityY();
            SetBehaviorState(BehaviorState::Clinging);
            break;
        }

        if (CheckJump())
            break;

        if (mTryingToTriggerVentania && mLandedAfterVentania)
        {
            TriggerVentania();
            break;
        }

        if (CheckHit())
            break;

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        Move(1.3f);

        break;

    case BehaviorState::Moving:
    {
        if (CheckJump())
            break;
        if (CheckDodge())
            break;
        if (CheckHit())
            break;
        if (CheckNevasca())
            break;

        if (!mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Jumping);
            break;
        }

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        if (!isCutscene && mInputMovementDir.x == 0.f && mInputMovementDir.y == 0.f)
        {
            SetBehaviorState(BehaviorState::Idle);
            break;
        }

        if (isCutscene && mRigidBodyComponent->GetVelocity().x == 0.f && mRigidBodyComponent->GetVelocity().y == 0.f)
        {
            SetBehaviorState(BehaviorState::Idle);
            break;
        }

        mTimerComponent->Restart(mCoyoteTimer);
        Move();
        SetLandedAfterVentania(true);

        break;
    }

    case BehaviorState::Idle:
    {
        if (CheckJump())
            break;

        if (CheckDodge())
            break;

        if (CheckHit())
            break;

        if (CheckNevasca())
            break;

        if (!mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Jumping);
            break;
        }

        if (!isCutscene && (mInputMovementDir.x != 0 || mInputMovementDir.y != 0))
        {
            SetBehaviorState(BehaviorState::Moving);
            break;
        }

        if (isCutscene && (mRigidBodyComponent->GetVelocity().x != 0.f || mRigidBodyComponent->GetVelocity().y != 0.f))
        {
            SetBehaviorState(BehaviorState::Moving);
            break;
        }

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        mTimerComponent->Restart(mCoyoteTimer);
        SetLandedAfterVentania(true);

        break;
    }

    case BehaviorState::Charging:
        if (!mRigidBodyComponent->GetOnGround())
        {
            mRigidBodyComponent->ResetVelocity();
            // no break here.
        }

        if (!mTryingToFireFireball)
        {
            SetBehaviorState(BehaviorState::Idle);
        }
        break;

    case BehaviorState::Dashing:
    {
        mRigidBodyComponent->SetApplyGravity(false); // onground check is diff when gravity is off.

        if (mTimerComponent->checkTimerRemaining(mDashGravityDisableTimer) <= 0.f)
        {
            mRigidBodyComponent->ResetVelocity();
            mRigidBodyComponent->SetApplyGravity(true);
            SetBehaviorState(BehaviorState::Falling);
            break;
        }

        break;
    }

    case BehaviorState::AerialAttacking:
    {
        if (mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Idle);
            mAerialAttackCollider->Dismiss();
            mAerialAttackCollider = nullptr;
            break;
        }

        break;
    }

    case BehaviorState::Clinging:
    {
        if (!IsPressingAgainstWall())
        {
            SetBehaviorState(BehaviorState::Falling);
            break;
        }

        if (mRigidBodyComponent->GetOnGround())
        {
            SetBehaviorState(BehaviorState::Idle);
            break;
        }

        if (mTryingToTriggerVentania && mLandedAfterVentania)
        {
            TriggerVentania();
            break;
        }

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        float mYSpeed = mRigidBodyComponent->GetVelocity().y;

        if (mYSpeed > 0.f) // compensate gravity while falling
        {
            mRigidBodyComponent->ApplyForce(Vector2(0.f, -GRAVITY * .97f));
        }

        int left = mColliderComponent->IsCloseToTileWallHorizontally(1.f) == -1;

        SetRotation(left ? 0.f : Math::Pi);

        SetLandedAfterVentania(true);

        break;
    }

    case BehaviorState::TakingDamage:
        break;

    case BehaviorState::Attacking:
        break;

    case BehaviorState::Dodging:
        break;

    default:
        SetBehaviorState(BehaviorState::Idle);
        break;
    }
}

void Zoe::OnUpdate(float deltaTime)
{
    if (mGame->GetGamePlayState() == Game::GamePlayState::Dialogue)
    {
        mDrawComponent->SetAnimation("idle");
        return;
    }

    ManageState();

    if (mRigidBodyComponent->GetVelocity().x > 0.0f)
    {
        SetRotation(mBehaviorState != BehaviorState::Clinging ? 0.0f : Math::Pi);
    }

    else if (mRigidBodyComponent->GetVelocity().x < 0.0f)
    {
        SetRotation(mBehaviorState != BehaviorState::Clinging ? Math::Pi : 0.0f);
    }

    mColliderComponent->MaintainInMap();
    ManageAnimations();

    // Nevasca
    bool wasFiringNevasca = mIsFiringNevasca;

    mIsFiringNevasca = mIsFiringNevasca && mIsTryingToNevasca;

    if (!mIsFiringNevasca && mNevascaSoundHandle.IsValid() &&
        mGame->GetAudio()->GetSoundState(mNevascaSoundHandle) == SoundState::Playing)
    {
        mGame->GetAudio()->StopSound(mNevascaSoundHandle);
    }

    else if (mIsFiringNevasca)
    {
        mNevascaTimer += deltaTime;

        if (!wasFiringNevasca)
        {
            mNevascaTimer = 0.f;
        }
    }

    // unblock all buttons
    for (auto &entry : mButtonBlocked)
    {
        UnblockButton(entry.first);
    }
}

void Zoe::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
    {
        if (mIsFiringNevasca)
        {
            mDrawComponent->SetAnimation("spraying");
            mDrawComponent->SetAnimFPS(6.f);
            break;
        }

        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(10.0f);
        break;
    }
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("run");
        mDrawComponent->SetAnimFPS(10.0f);
        break;
    case BehaviorState::Jumping:
        mDrawComponent->SetAnimation("jump");
        break;
    case BehaviorState::Falling:
        mDrawComponent->SetAnimation("jump");
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("hurt");
        mDrawComponent->SetAnimFPS(0.01f);
        break;
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("hurt");
        mDrawComponent->SetAnimFPS(14.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(32.0f);
        break;

    case BehaviorState::Attacking:
        mDrawComponent->SetAnimation("ground-crush");
        mDrawComponent->SetAnimFPS(12.0f);
        break;

    case BehaviorState::Dodging:
        mDrawComponent->SetAnimation("dodging");
        mDrawComponent->SetAnimFPS(1.75f);
        break;

    case BehaviorState::AerialAttacking:
        mDrawComponent->SetAnimation("aerial-crush");
        mDrawComponent->SetAnimFPS(12.0f);
        break;

    case BehaviorState::Clinging:
        mDrawComponent->SetAnimation("clinging");
        mDrawComponent->SetAnimFPS(8.f);
        break;

    case BehaviorState::Dashing:
        mDrawComponent->SetAnimation("jump");
        break;

    default:
        break;
    }
}

void Zoe::Kill()
{
    if (mBehaviorState == BehaviorState::Dead)
        return;

    SetBehaviorState(BehaviorState::Dead);

    int maxDeaths = mGame->GetConfig()->Get<int>("ZOE.MAX_DEATHS");

    if (mDeaths >= maxDeaths || GetCurrentCheckpoint() == nullptr)
    {
        mGame->SetGameScene(Game::GameScene::DeathScreen);
        return;
    }

    mDeaths++;
    mGame->SetCameraCenterToShake(0.4f, 5);
    mGame->GetAudio()->PlaySound("respawn.wav");
    SetPosition(GetCurrentCheckpoint()->position - GetHalfSize());
    SetLifes(mGame->GetConfig()->Get<int>("ZOE.LIFE_POINTS"));
    SetBehaviorState(BehaviorState::Idle);
}

void Zoe::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::EnemyProjectile)
    {
        TakeDamage();
        TakeKnockback(Vector2(Math::Sign(-minOverlap), 0.f) * mGame->GetConfig()->Get<float>("ZOE.KNOCKBACK_FORCE"));
        return;
    }

    if (other->GetLayer() == ColliderLayer::SithAttack1)
    {
        TakeSithAttack1(minOverlap, other);
        return;
    }

    if (other->GetLayer() == ColliderLayer::SithAttack2)
    {
        TakeSithAttack2(minOverlap, other);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Quasar)
    {
        TakeDamage();

        float knockbackForce = mGame->GetConfig()->Get<float>("QUASAR.SPIKE_KNOCKBACK_FORCE");
        Quasar *quasar = static_cast<Quasar *>(other->GetOwner());

        if (quasar->GetBehaviorState() == BehaviorState::Attacking)
        {
            TakeKnockback(Vector2(Math::Sign(-minOverlap), -1) * knockbackForce);
        }
        else
        {
            TakeKnockback(Vector2(Math::Sign(-minOverlap) * .7f, -.2f) * knockbackForce);
        }

        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        TakeDamage();
        TakeKnockback(Vector2(Math::Sign(-minOverlap), 0.f) * mGame->GetConfig()->Get<float>("ZOE.KNOCKBACK_FORCE"));
        return;
    }

    Actor::OnHorizontalCollision(minOverlap, other);
}

void Zoe::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (
        other->GetLayer() == ColliderLayer::EnemyProjectile ||
        other->GetLayer() == ColliderLayer::Enemy)
    {
        Vector2 dist = GetCenter() - other->GetCenter();
        dist.Normalize();

        TakeKnockback(dist * mGame->GetConfig()->Get<float>("ZOE.KNOCKBACK_FORCE"));

        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::SithAttack1)
    {
        TakeSithAttack1(minOverlap, other);
        return;
    }

    if (other->GetLayer() == ColliderLayer::SithAttack2)
    {
        TakeSithAttack2(minOverlap, other);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Quasar)
    {
        TakeDamage();

        float knockbackForce = mGame->GetConfig()->Get<float>("QUASAR.SPIKE_KNOCKBACK_FORCE");
        TakeKnockback(Vector2(1.f, Math::Sign(-minOverlap)) * knockbackForce);

        return;
    }

    Actor::OnVerticalCollision(minOverlap, other);
}

void Zoe::AnimationEndCallback(std::string animationName)
{
    if (animationName == "hurt")
    {
        SetBehaviorState(BehaviorState::Idle);
        return;
    }

    if (animationName == "charging" && mTryingToFireFireball)
    {
        FireFireball();

        SetBehaviorState(BehaviorState::Idle);
        return;
    }

    if (animationName == "dodging")
    {
        DodgeEnd();
        return;
    }

    if (animationName == "ground-crush")
    {
        SetBehaviorState(BehaviorState::Idle);
        mAttackCollider->Dismiss();
        mAttackCollider = nullptr;
        return;
    }

    if (animationName == "aerial-crush")
    {
        SetBehaviorState(BehaviorState::Jumping);
        mAerialAttackCollider->Dismiss();
        mAerialAttackCollider = nullptr;
        return;
    }
}

bool Zoe::CheckFireballOnCooldown()
{
    return mFireballCooldownTimer != nullptr && mTimerComponent->checkTimerRemaining(mFireballCooldownTimer) > 0.f;
}

float Zoe::GetFireballCooldownProgress()
{
    if (mFireballCooldownTimer)
    {
        float cooldown = mGame->GetConfig()->Get<float>("ZOE.POWERS.FIREBALL.COOLDOWN");
        return mTimerComponent->checkTimerRemaining(mFireballCooldownTimer) / cooldown;
    }

    return 1.f;
}

void Zoe::OnDamageCallback()
{
    mGame->SetCameraCenterToShake(0.125f, 1.5f);

    if (
        !mDamageSoundHandle.IsValid() ||
        mGame->GetAudio()->GetSoundState(mDamageSoundHandle) == SoundState::Stopped)
    {
        mDamageSoundHandle = mGame->GetAudio()->PlaySound("zoeTakeDamage.wav");
    }

    SetInvincibilityOn();
    mTimerComponent->AddTimer(0.75f, [this]()
                              { SetInvincibilityOff(); });
}

void Zoe::TakeDamage()
{
    if (mBehaviorState == BehaviorState::Dodging)
    {
        return;
    }

    Actor::TakeDamage();
}

void Zoe::TakeSithAttack1(const float minOverlap, AABBColliderComponent *other)
{
    Collider *sithAttackCollider = static_cast<Collider *>(other->GetOwner());
    Sith *sith = static_cast<Sith *>(sithAttackCollider->GetOwnerActor());

    float xDiff = Math::Sign(GetCenter().x - sith->GetCenter().x);

    TakeDamage();
    TakeKnockback(Vector2(xDiff, 1.f) * 2 * mGame->GetConfig()->Get<float>("ZOE.KNOCKBACK_FORCE"));
    return;
}

void Zoe::TakeSithAttack2(const float minOverlap, AABBColliderComponent *other)
{
    Collider *sithAttackCollider = static_cast<Collider *>(other->GetOwner());
    Sith *sith = static_cast<Sith *>(sithAttackCollider->GetOwnerActor());

    float xDiff = Math::Sign(GetCenter().x - sith->GetCenter().x);

    TakeDamage();
    TakeKnockback(Vector2(xDiff, 1.f) * 3.5 * mGame->GetConfig()->Get<float>("ZOE.KNOCKBACK_FORCE"));
    return;
}

bool Zoe::IsAbilitiesLocked() const
{
    return mAbilitiesLocked;
}

void Zoe::SetAbilitiesLocked(bool locked)
{
    mAbilitiesLocked = locked;
}

bool Zoe::IsMovementLocked() const
{
    return mMovementLocked;
}

void Zoe::SetMovementLocked(bool locked)
{
    mMovementLocked = locked;
}

void Zoe::Move(float modifier)
{
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing)
        return;

    Vector2 movementDir = mInputMovementDir;

    if (mGame->GetApplyGravityScene() && mRigidBodyComponent->GetApplyGravity())
    {
        movementDir.y = 0.f;
        movementDir.Normalize();
    }

    float MAX_MOVE_X_SPEED_ON_AIR = 120.f;
    float currentVelX = mRigidBodyComponent->GetVelocity().x;

    if (!mRigidBodyComponent->GetOnGround() &&
        Math::Abs(currentVelX) >= MAX_MOVE_X_SPEED_ON_AIR &&
        Math::Sign(currentVelX) == Math::Sign(movementDir.x))
    {
        return;
    }

    mRigidBodyComponent->ApplyForce(
        movementDir * mForwardSpeed * modifier);
}

int Zoe::IsPressingAgainstWall()
{
    int closeToWall = mColliderComponent->IsCloseToTileWallHorizontally(1.f);

    if (closeToWall == 0)
        return 0;

    if (closeToWall == -1 && mInputMovementDir.x < 0.f)
        return -1;

    if (closeToWall == 1 && mInputMovementDir.x > 0.f)
        return 1;

    return 0;
}

void Zoe::SetCheckpoint(const Vector2 &position)
{
    if (mCurrentCheckpoint == nullptr)
    {
        mCurrentCheckpoint = new Checkpoint(position);
        return;
    }

    mCurrentCheckpoint->position = position;
}

Checkpoint *Zoe::GetCurrentCheckpoint() const
{
    return mCurrentCheckpoint;
}

void Zoe::SetMana(float mana)
{
    mMana = mana;

    if (mMana < 0.f)
        mMana = 0.f;

    if (mMana > mGame->GetConfig()->Get<float>("ZOE.MAX_MANA"))
        mMana = mGame->GetConfig()->Get<float>("ZOE.MAX_MANA");
}

void Zoe::ConsumeMana(float amount)
{
    SetMana(mMana - amount);
    mConsumedManaThisFrame = true;
}

void Zoe::RegenerateMana()
{
    if (mConsumedManaThisFrame)
    {
        mConsumedManaThisFrame = false; // this logic could be in update loop also.
        return;
    }

    SetMana(mMana + mGame->GetConfig()->Get<float>("ZOE.MANA_REGEN_RATE_PER_SECOND"));
}