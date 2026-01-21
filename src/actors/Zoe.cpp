#include "Zoe.h"

Zoe::Zoe(
    Game *game, const float forwardSpeed, const Vector2 &center)
    : Actor(game, 6, true), mForwardSpeed(forwardSpeed),
      mTryingToFireFireball(false), mFireballCooldownTimer(), mDodgeCooldownTimer(),
      mIsVentaniaOnCooldown(false), mTryingToTriggerVentania(false),
      mIsTryingToHit(false), mAttackCollider(nullptr), mIsTryingToDodge(false),
      mInputMovementDir(0.f, 0.f), mMovementLocked(false), mAbilitiesLocked(false),
      mDamageSoundHandle(SoundHandle::Invalid), mIsTryingToJump(false), mNevascaSoundHandle(SoundHandle::Invalid),
      mIsTryingToNevasca(false), mIsFiringNevasca(false), mNevascaTimer(0.f)
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
    mDrawComponent->AddAnimation("hurt", {26});
    mDrawComponent->AddAnimation("dodging", {27});
    mDrawComponent->AddAnimation("aerial-crush", 28, 37);
    mDrawComponent->AddAnimation("clinging", {38, 39, 40});
    mDrawComponent->AddAnimation("charging", 41, 46);

    mDrawComponent->SetAnimation("idle");

    mColliderComponent->SetIgnoreLayers(
        Zoe::IGNORED_LAYERS_DEFAULT);

    SetPosition(center - GetHalfSize());

    mGame->SetZoe(this);

    mAerialAttackCollider = new Collider(
        mGame,
        this,
        GetCenter() - Vector2(20, 30),
        Vector2(40, 40),
        [this](bool collided, const float minOverlap, AABBColliderComponent *other) {},
        DismissOn::None,
        ColliderLayer::PlayerAttack,
        {ColliderLayer::Player},
        1.f);

    mAerialAttackCollider->SetEnabled(false);

    mCoyoteTimer = mTimerComponent->AddNotRemovableTimer(0.1f, nullptr);
}

Zoe::~Zoe()
{
    mGame->SetZoe(nullptr);
}

void Zoe::OnProcessInput(const uint8_t *state)
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

    if (mAbilitiesLocked)
    {
        mIsTryingToDodge = false;
        mIsTryingToJump = false;
        mIsTryingToHit = false;
        mTryingToFireFireball = false;
        mTryingToTriggerVentania = false;
        mIsTryingToNevasca = false;
    }

    else
    {
        mIsTryingToHit = SDL_GameControllerGetButton(controller, Zoe::HIT_BUTTON);
        mTryingToFireFireball = SDL_GameControllerGetButton(controller, Zoe::FIREBALL_BUTTON);
        mTryingToTriggerVentania = SDL_GameControllerGetButton(controller, Zoe::VENTANIA_BUTTON);
        mIsTryingToDodge = SDL_GameControllerGetButton(controller, Zoe::DODGE_BUTTON);
        mIsTryingToJump = SDL_GameControllerGetButton(controller, Zoe::JUMP_BUTTON);
        mIsTryingToNevasca = SDL_GameControllerGetAxis(controller, Zoe::NEVASCA_AXIS) > 0;
    }

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

    switch (mBehaviorState)
    {
    case BehaviorState::Dying:
        Kill();
        break;

    case BehaviorState::Jumping:
        if (mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Idle;
        }

        if ( IsPressingAgainstWall() != 0 )
        {
            mBehaviorState = BehaviorState::Clinging;
            break;
        }

        if (mRigidBodyComponent->GetVelocity().y > 0.f)
        {
            mBehaviorState = BehaviorState::Falling;
        }

        if (mTryingToTriggerVentania && !mIsVentaniaOnCooldown)
        {
            TriggerVentania();
            break;
        }

        if (CheckHit()) break;

        Move(.125f);

        break;

    case BehaviorState::Falling:
        if ( CheckJump() ) break;

        if (mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        if ( IsPressingAgainstWall() != 0 )
        {
            mBehaviorState = BehaviorState::Clinging;
            break;
        }

        if (mTryingToTriggerVentania && !mIsVentaniaOnCooldown)
        {
            TriggerVentania();
            break;
        }

        if (CheckHit()) break;

        break;

    case BehaviorState::Moving:
    {
        if ( CheckJump() ) break;
        if ( CheckDodge() ) break;
        if ( CheckHit() ) break;

        if ( CheckNevasca() ) 
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        if (!mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (!isCutscene && mInputMovementDir.x == 0.f && mInputMovementDir.y == 0.f) 
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        if (isCutscene && mRigidBodyComponent->GetVelocity().x == 0.f && mRigidBodyComponent->GetVelocity().y == 0.f)
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        mTimerComponent->Restart(mCoyoteTimer);
        Move();

        break;
    }

    case BehaviorState::Idle:
    {
        if ( CheckJump() ) break;

        if ( CheckDodge() ) break;

        if ( CheckHit() ) break;

        if ( CheckNevasca() ) break;

        if (!mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        if (!isCutscene && (mInputMovementDir.x != 0 || mInputMovementDir.y != 0))
        {
            mBehaviorState = BehaviorState::Moving;
            break;
        }

        if (isCutscene && (mRigidBodyComponent->GetVelocity().x != 0.f || mRigidBodyComponent->GetVelocity().y != 0.f))
        {
            mBehaviorState = BehaviorState::Moving;
            break;
        }

        if (mTryingToFireFireball && !CheckFireballOnCooldown())
        {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        mTimerComponent->Restart(mCoyoteTimer);

        break;
    }

    case BehaviorState::Charging:
        if (!mTryingToFireFireball)
        {
            mBehaviorState = BehaviorState::Idle;
        }
        break;

    case BehaviorState::TakingDamage:
        break;

    case BehaviorState::Attacking:
        break;

    case BehaviorState::Dodging:
        break;

    case BehaviorState::AerialAttacking:
    {
        if (!mAerialAttackCollider->IsEnabled())
        {
            mAerialAttackCollider->SetEnabled(true);
        }

        Vector2 attackColliderPosition = GetCenter() - Vector2(20, 30);
        mAerialAttackCollider->SetPosition(attackColliderPosition);
        
        if (mRigidBodyComponent->GetOnGround())
        {
            EndAerialAttack();
        }

        break;
    }

    case BehaviorState::Clinging: {
        if (mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        if (mTryingToTriggerVentania && !mIsVentaniaOnCooldown)
        {
            TriggerVentania();
            break;
        }
        
        if (IsPressingAgainstWall() == 0)
        {
            mBehaviorState = BehaviorState::Falling;
            break;
        }

        mRigidBodyComponent->ApplyForce(Vector2(0.f, -GRAVITY * .4f));

        break;
    }

    default:
        mBehaviorState = BehaviorState::Idle;
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
        
        if (!wasFiringNevasca) {
            mNevascaTimer = 0.f;
        }
    }
}

void Zoe::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(10.0f);
        break;
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("run");
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
        mDrawComponent->SetAnimFPS(4.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(8.0f);

        break;

    case BehaviorState::Attacking:
        mDrawComponent->SetAnimation("ground-crush");
        mDrawComponent->SetAnimFPS(12.0f);

        if (mDrawComponent->GetCurrentSprite() == 5 && mAttackCollider == nullptr)
        {
            mAttackCollider = new Collider(
                mGame,
                this,
                GetCenter() + (GetRotation() == 0.f ? Vector2(11, -3) : Vector2(-22, -6)),
                Vector2(14, 14),
                [this](bool collided, const float minOverlap, AABBColliderComponent *other) {},
                DismissOn::Both,
                ColliderLayer::PlayerAttack,
                {ColliderLayer::Player},
                .5f);
        }
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

    default:
        break;
    }
}

void Zoe::Kill()
{
    mGame->SetGameScene(Game::GameScene::DeathScreen);
}

void Zoe::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::EnemyProjectile)
    {
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
        TakeDamage(Vector2(Math::Sign(-minOverlap), 1.f) * Zoe::DEFAULT_KNOCKBACK_FORCE * 6.f);
        return;
    }

    Actor::OnHorizontalCollision(minOverlap, other);
}

void Zoe::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::EnemyProjectile)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy && minOverlap > 0.f)
    {
        float ySpeed = mRigidBodyComponent->GetJumpImpulseY(3);
        mRigidBodyComponent->ApplyImpulse(Vector2(0.f, ySpeed));
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy && minOverlap < 0.f)
    {
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

    Actor::OnVerticalCollision(minOverlap, other);
}

void Zoe::AnimationEndCallback(std::string animationName)
{
    if (animationName == "hurt")
    {
        mBehaviorState = BehaviorState::Idle;
        mInvincible = false;
        return;
    }

    if (animationName == "charging" && mTryingToFireFireball)
    {
        FireFireball();
        
        mBehaviorState = BehaviorState::Idle;
        return;
    }

    if (animationName == "ground-crush")
    {
        mBehaviorState = BehaviorState::Idle;
        mAttackCollider->Dismiss();
        mAttackCollider = nullptr;
        return;
    }

    if (animationName == "dodging")
    {
        DodgeEnd();
        return;
    }

    if (animationName == "aerial-crush")
    {
        EndAerialAttack();
        return;
    }
}

void Zoe::FireFireball()
{
    if (CheckFireballOnCooldown())
        return;

    Vector2 fireballDir = mInputMovementDir;

    if (fireballDir.LengthSq() == 0.f)
    {
        fireballDir = GetForward();
    }

    auto projectile = new Fireball(
        mGame,
        GetPosition() + GetFireballOffset(),
        fireballDir,
        Zoe::FIREBALL_SPEED,
        this);

    mFireballCooldownTimer = mTimerComponent->AddTimer(Zoe::FIREBALL_COOLDOWN, [this]() {});
    mGame->GetAudio()->PlaySound("fireball.wav");
}

void Zoe::TriggerVentania()
{
    if (mIsVentaniaOnCooldown)
        return;

    mRigidBodyComponent->ResetVelocity();

    Vector2 ventaniaDir = mInputMovementDir.LengthSq() > 0.f ? mInputMovementDir : Vector2(0.f, 1.f);

    mRigidBodyComponent->ApplyForce(ventaniaDir * Zoe::VETANIA_SPEED);

    new Ventania(
        GetGame(),
        GetCenter(),
        ventaniaDir);

    SetVentaniaOnCooldown(true);
    mTimerComponent->AddTimer(Zoe::VETANIA_COOLDOWN, [this]()
                              { SetVentaniaOnCooldown(false); });

    mGame->GetAudio()->PlaySound("ventania.wav");
}

void Zoe::TakeDamage(const Vector2 &knockback)
{
    if (mBehaviorState == BehaviorState::Dodging)
    {
        return;
    }

    Actor::TakeDamage(knockback);

    if (!mDamageSoundHandle.IsValid() ||
        mGame->GetAudio()->GetSoundState(mDamageSoundHandle) == SoundState::Stopped)
    {
        mDamageSoundHandle = mGame->GetAudio()->PlaySound("zoeTakeDamage.wav");
    }
}

bool Zoe::CheckJump()
{
    if (!mGame->GetApplyGravityScene())
        return false;
    
    if (!mIsTryingToJump)
        return false;

    if (mBehaviorState == BehaviorState::Jumping)
        return false;
    
    bool coyoteExpired = mTimerComponent->checkTimerRemaining(mCoyoteTimer) <= 0.f;

    if (mBehaviorState == BehaviorState::Falling && coyoteExpired)
        return false;

    float jumpForce = mRigidBodyComponent->GetJumpImpulseY(5);
    float horizontalDir = Math::Sign(mRigidBodyComponent->GetVelocity().x);

    mRigidBodyComponent->ApplyImpulse(
        Vector2(horizontalDir * 10.f, jumpForce));

    mBehaviorState = BehaviorState::Jumping;

    return true;
}

void Zoe::TakeSithAttack1(const float minOverlap, AABBColliderComponent *other)
{
    Collider *sithAttackCollider = static_cast<Collider *>(other->GetOwner());
    Sith *sith = static_cast<Sith *>(sithAttackCollider->GetOwnerActor());

    float xDiff = Math::Sign(GetCenter().x - sith->GetCenter().x);

    float modifier = 2.f * Zoe::DEFAULT_KNOCKBACK_FORCE;

    TakeDamage(Vector2(xDiff, 1.f) * modifier);
    return;
}

void Zoe::TakeSithAttack2(const float minOverlap, AABBColliderComponent *other)
{
    Collider *sithAttackCollider = static_cast<Collider *>(other->GetOwner());
    Sith *sith = static_cast<Sith *>(sithAttackCollider->GetOwnerActor());

    float xDiff = Math::Sign(GetCenter().x - sith->GetCenter().x);

    float modifier = 5.f * Zoe::DEFAULT_KNOCKBACK_FORCE;

    TakeDamage(Vector2(xDiff, 1.f) * modifier);
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

bool Zoe::CheckDodge()
{
    if (!mIsTryingToDodge || mBehaviorState == BehaviorState::Dodging)
        return false;

    if (mDodgeCooldownTimer != nullptr && mTimerComponent->checkTimerRemaining(mDodgeCooldownTimer) > 0.f)
    {
        return false;
    }

    mBehaviorState = BehaviorState::Dodging;
    mColliderComponent->SetIgnoreLayers(Zoe::IGNORED_LAYERS_DODGE);
    mColliderComponent->SetBB(&DODGE_BB);

    return true;
}

void Zoe::DodgeEnd()
{
    if (mBehaviorState != BehaviorState::Dodging)
        return;

    mBehaviorState = BehaviorState::Idle;

    Vector2 currentPos = GetPosition();
    SetPosition(Vector2(currentPos.x, currentPos.y - 5));

    mColliderComponent->SetIgnoreLayers(Zoe::IGNORED_LAYERS_DEFAULT);
    mColliderComponent->SetBB(&DEFAULT_BB);

    mDodgeCooldownTimer = mTimerComponent->AddTimer(Zoe::DODGE_COOLDOWN, nullptr);
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

    float MAX_MOVE_X_SPEED_ON_AIR = 50.f;
    float currentVelX = mRigidBodyComponent->GetVelocity().x;

    if (!mRigidBodyComponent->GetOnGround() && 
        Math::Abs(currentVelX) >= MAX_MOVE_X_SPEED_ON_AIR &&
        Math::Sign(currentVelX) == Math::Sign(movementDir.x)
    )
    {
        return;
    }

    mRigidBodyComponent->ApplyForce(
        movementDir * mForwardSpeed * modifier);
}

void Zoe::EndAerialAttack()
{
    mAerialAttackCollider->SetEnabled(false);
    mBehaviorState = BehaviorState::Jumping;
}

bool Zoe::CheckHit()
{
    if (!mIsTryingToHit) return false;
    if (mBehaviorState == BehaviorState::AerialAttacking) return false;
    if (mBehaviorState == BehaviorState::Attacking) return false;
    
    bool onGround = mRigidBodyComponent->GetOnGround();

    mBehaviorState = onGround ? BehaviorState::Attacking : BehaviorState::AerialAttacking;
    mGame->GetAudio()->PlaySound("zoeSmash.wav");

    if (!onGround)
    {
        return true;
    }
        
    float ySpeed = mRigidBodyComponent->GetJumpImpulseY(.5f);
    mRigidBodyComponent->ApplyImpulse(Vector2(0.f, ySpeed));

    return true;
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
        Vector2::RotateVec(nevascaDir, -dissipationAngle)
    };

    while (mNevascaTimer >= rate)
    {
        mNevascaTimer -= rate;
        
        for (const Vector2& dir : dirs)
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

    return true;
}