#include "Zoe.h"
#include "Tile.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"
#include "../components/TimerComponent.h"
#include "./Collider.h"
#include "./traps/Spikes.h"
#include "./traps/Spear.h"
#include "./traps/Shuriken.h"
#include "../libs/Math.h"

Ventania::Ventania(Game *game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed) : Actor(game)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Ventania/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Ventania/texture.json";

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Ventania::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player) - 1);

    mDrawAnimatedComponent->AddAnimation("normal", 0, 5);
    mDrawAnimatedComponent->SetAnimation("normal");
    mDrawAnimatedComponent->SetAnimFPS(10.f);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);

    Vector2 playerFeet = playerCenter;

    SetPosition(playerCenter - mDrawAnimatedComponent->GetHalfSpriteSize());

    float directionAngle = Math::Atan2(playerMoveDir.y, playerMoveDir.x);
    float originalAngle = Math::Atan2(-1.f, 0.f);
    directionAngle -= originalAngle;

    SetRotation(directionAngle);
}

void Ventania::AnimationEndCallback(std::string animationName)
{
    if (animationName == "normal")
    {
        SetState(ActorState::Destroy);
    }
}

//

Fireball::Fireball(
    class Game *game, Vector2 position,
    Vector2 dir, float speed, Actor *shooter
) : Projectile(game, position, Vector2(0.f,0.f), speed, shooter), mRicochetsCount(0)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Fireball/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Fireball/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);

    mColliderComponent = new AABBColliderComponent(
        this,
        45, 28,
        10, 9,
        ColliderLayer::Fireball);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Fireball::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Enemy) - 1);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 28);
    mDrawAnimatedComponent->AddAnimation("dying", 29, 48);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - mDrawAnimatedComponent->GetHalfSpriteSize());

    mDirection = dir;
    mDirection.Normalize();

    float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
    float originalAngle = Math::Atan2(0.f, 1.f);
    directionAngle -= originalAngle;

    SetRotation(directionAngle);
}

void Fireball::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying")
    {
        SetState(ActorState::Destroy);
    }
}

void Fireball::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying)
    {
        mDrawAnimatedComponent->SetAnimation("dying");
        mDrawAnimatedComponent->SetAnimFPS(10.f);
    }
    else if (mBehaviorState == BehaviorState::Moving)
    {
        mDrawAnimatedComponent->SetAnimation("flying");
        mDrawAnimatedComponent->SetAnimFPS(20.f);
    }
}

void Fireball::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (mBehaviorState != BehaviorState::Moving)
        return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        return;
    }

    if (mRicochetsCount >= MAX_RICOCHETS || other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mRicochetsCount++;

        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(Math::Sign(minOverlap), 0.f));

        reflection.Normalize();

        mDirection = reflection;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;

        SetRotation(directionAngle);
    }
}

void Fireball::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (mBehaviorState != BehaviorState::Moving)
        return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        return;
    }

    if (mRicochetsCount >= MAX_RICOCHETS || other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mRicochetsCount++;

        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(0.f, Math::Sign(minOverlap)));

        reflection.Normalize();

        mDirection = reflection;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;

        SetRotation(directionAngle);
    }
}

void Fireball::Kill()
{
    Projectile::Kill();

    // mColliderComponent->SetEnabled(false);
}

//

Zoe::Zoe(
    Game *game, const float forwardSpeed, const Vector2 &center
)
    : Actor(game, 6, true), mForwardSpeed(forwardSpeed),
      mTryingToFireFireball(false), mIsFireballOnCooldown(false),
      mIsVentaniaOnCooldown(false), mTryingToTriggerVentania(false),
      mIsTryingToHit(false), mAttackCollider(nullptr), mIsTryingToDodge(false),
      mInputMovementDir(0.f, 0.f), mMovementLocked(false), mAbilitiesLocked(false),
      mDamageSoundHandle(SoundHandle::Invalid)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 11.0f);

    mColliderComponent = new AABBColliderComponent(this, 27, 39, 13, 24,
                                                   ColliderLayer::Player);

    mTimerComponent = new TimerComponent(this);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Zoe/texture.png",
        "../assets/Sprites/Zoe/texture.json",
        std::bind(&Zoe::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("idle", 0, 8);
    mDrawComponent->AddAnimation("ground-crush", 9, 16);
    mDrawComponent->AddAnimation("blink", 17, 20);
    mDrawComponent->AddAnimation("jump", 20, 22);
    mDrawComponent->AddAnimation("run", 23, 28);
    mDrawComponent->AddAnimation("hurt", {29});
    mDrawComponent->AddAnimation("dodging", {30});
    mDrawComponent->AddAnimation("aerial-crush", 31, 40);

    mDrawComponent->SetAnimation("idle");

    mColliderComponent->SetIgnoreLayers(
        Zoe::IGNORED_LAYERS_DEFAULT);

    SetPosition(center - GetHalfSize());

    mGame->SetZoe(this);
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
    const bool hasController = controller && SDL_GameControllerGetAttached(controller);

    if (!mAbilitiesLocked)
    {
        mIsTryingToHit = state[Zoe::HIT_KEY];
        mTryingToFireFireball = state[Zoe::FIREBALL_KEY];
        mTryingToTriggerVentania = state[Zoe::VENTANIA_KEY];
        mIsTryingToDodge = state[Zoe::DODGE_KEY];
        mIsTryingToJump = state[Zoe::JUMP_KEY];
    }

    if (hasController && !mAbilitiesLocked)
    {
        mIsTryingToHit = mIsTryingToHit || SDL_GameControllerGetButton(controller, Zoe::HIT_BUTTON);
        mTryingToFireFireball = mTryingToFireFireball || SDL_GameControllerGetButton(controller, Zoe::FIREBALL_BUTTON);
        mTryingToTriggerVentania = mTryingToTriggerVentania || SDL_GameControllerGetButton(controller, Zoe::VENTANIA_BUTTON);
        mIsTryingToDodge = mIsTryingToDodge || SDL_GameControllerGetButton(controller, Zoe::DODGE_BUTTON);
        mIsTryingToJump = mIsTryingToJump || SDL_GameControllerGetButton(controller, Zoe::JUMP_BUTTON);
    }

    if (mMovementLocked)
    {
        mInputMovementDir = Vector2(0.f, 0.f);
        return;
    }

    // Keyboard movement
    float kbX = static_cast<float>(state[SDL_SCANCODE_D] - state[SDL_SCANCODE_A]);
    float kbY = static_cast<float>(state[SDL_SCANCODE_S] - state[SDL_SCANCODE_W]);

    // Controller movement
    float padX = 0.0f;
    float padY = 0.0f;

    if (hasController)
    {
        int rawX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int rawY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        const int DEADZONE = 2000;

        if (SDL_abs(rawX) < DEADZONE)
            rawX = 0;
        if (SDL_abs(rawY) < DEADZONE)
            rawY = 0;

        const float MAX_AXIS = 32767.0f;

        padX = rawX / MAX_AXIS;
        padY = rawY / MAX_AXIS;
    }

    float finalX = kbX != 0.0f ? kbX : padX;
    float finalY = kbY != 0.0f ? kbY : padY;

    mInputMovementDir = Vector2(finalX, finalY);

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

        if (mTryingToTriggerVentania && !mIsVentaniaOnCooldown)
        {
            TriggerVentania();
            break;
        }

        if (mIsTryingToHit)
        {
            mBehaviorState = BehaviorState::AerialAttacking;
            mGame->GetAudio()->PlaySound("zoeSmash.wav");
            float upwardForce = mRigidBodyComponent->GetVerticalForce(.3f);
            mRigidBodyComponent->ApplyForce(Vector2(0.f, upwardForce));
            break;
        }

        break;

    case BehaviorState::Moving:
    {
        if (mIsTryingToJump && mGame->GetApplyGravityScene())
        {
            float jumpForce = mRigidBodyComponent->GetVerticalForce(3);
            mRigidBodyComponent->ApplyForce(Vector2(0.f, jumpForce));
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        if (mIsTryingToDodge)
        {
            mBehaviorState = BehaviorState::Dodging;
            break;
        }

        if (mIsTryingToHit)
        {
            mBehaviorState = BehaviorState::Attacking;
            mGame->GetAudio()->PlaySound("zoeSmash.wav");
            break;
        }

        if (!mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        if (mTryingToFireFireball && !mIsFireballOnCooldown)
        {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (mGame->GetGamePlayState() == Game::GamePlayState::Playing)
        {
            Vector2 movementDir = mInputMovementDir;

            if (mGame->GetApplyGravityScene() && mRigidBodyComponent->GetApplyGravity())
            {
                movementDir.y = 0.f;
                movementDir.Normalize();
            }

            mRigidBodyComponent->ApplyForce(
                movementDir * mForwardSpeed);
        }

        if (
            mRigidBodyComponent->GetVelocity().x == 0.f &&
            mRigidBodyComponent->GetVelocity().y == 0.f)
        {
            mBehaviorState = BehaviorState::Idle;
            break;
        }

        break;
    }

    case BehaviorState::Idle:
    {
        if (mIsTryingToJump && mGame->GetApplyGravityScene())
        {
            float jumpForce = mRigidBodyComponent->GetVerticalForce(3);
            mRigidBodyComponent->ApplyForce(Vector2(0.f, jumpForce));
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        if (mIsTryingToDodge)
        {
            mBehaviorState = BehaviorState::Dodging;
            break;
        }

        if (mIsTryingToHit)
        {
            mBehaviorState = BehaviorState::Attacking;
            mGame->GetAudio()->PlaySound("zoeSmash.wav");
            break;
        }

        if (!mRigidBodyComponent->GetOnGround())
        {
            mBehaviorState = BehaviorState::Jumping;
            break;
        }

        Vector2 movementDir = mInputMovementDir;

        if (
            movementDir.x != 0 || movementDir.x != 0)
        {
            mBehaviorState = BehaviorState::Moving;
            break;
        }

        if (mTryingToFireFireball && !mIsFireballOnCooldown)
        {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

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
        mColliderComponent->SetIgnoreLayers(
            Zoe::IGNORED_LAYERS_DODGE);
        break;

    case BehaviorState::AerialAttacking:
        break;

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
        SetRotation(0.0f);
    }

    else if (mRigidBodyComponent->GetVelocity().x < 0.0f)
    {
        SetRotation(Math::Pi);
    }

    mColliderComponent->MaintainInMap();
    ManageAnimations();
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
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("hurt");
        mDrawComponent->SetAnimFPS(4.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("blink");
        mDrawComponent->SetAnimFPS(4.0f);

        if (mDrawComponent->GetCurrentSprite() == 3)
        {
            FireFireball();
        }

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
                Vector2(9, 9),
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

    if (other->GetLayer() == ColliderLayer::Spikes)
    {
        Collider *spikeCollider = static_cast<Collider*>(other->GetOwner());
        Spikes *spikes = static_cast<Spikes*>(spikeCollider->GetOwnerActor());
        TakeSpikeHit(spikes->GetBaseCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::SpearTip)
    {
        Collider *spearTipCollider = static_cast<Collider*>(other->GetOwner());
        Spear *spear = static_cast<Spear*>(spearTipCollider->GetOwnerActor());
        TakeSpearHit(spear->GetTipCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::Shuriken) //shuriken doesnt use intermediate collider
    {
        Shuriken *shuriken = static_cast<Shuriken*>(other->GetOwner());
        TakeShurikenHit(shuriken->GetCenter());
        return;
    }
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
        mRigidBodyComponent->ApplyForce(
            Vector2(
                0.f,
                mRigidBodyComponent->GetVerticalForce(3)));
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy && minOverlap < 0.f)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::Spikes)
    {
        Collider *spikeCollider = static_cast<Collider*>(other->GetOwner());
        Spikes *spikes = static_cast<Spikes*>(spikeCollider->GetOwnerActor());
        TakeSpikeHit(spikes->GetBaseCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::SpearTip)
    {
        Collider *spearTipCollider = static_cast<Collider*>(other->GetOwner());
        Spear *spear = static_cast<Spear*>(spearTipCollider->GetOwnerActor());
        TakeSpearHit(spear->GetTipCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::Shuriken) //shuriken doesnt use intermediate collider
    {
        Shuriken *shuriken = static_cast<Shuriken*>(other->GetOwner());
        TakeShurikenHit(shuriken->GetCenter());
        return;
    }
}

void Zoe::AnimationEndCallback(std::string animationName)
{
    if (animationName == "hurt")
    {
        mBehaviorState = BehaviorState::Idle;
        mInvincible = false;
        return;
    }

    if (animationName == "blink" && mTryingToFireFireball)
    {
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
        mBehaviorState = BehaviorState::Idle;
        mColliderComponent->SetIgnoreLayers(
            Zoe::IGNORED_LAYERS_DEFAULT);
        return;
    }

    if (animationName == "aerial-crush")
    {
        mBehaviorState = BehaviorState::Jumping;
        return;
    }
}

void Zoe::FireFireball()
{
    if (mIsFireballOnCooldown)
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

    SetFireballOnCooldown(true);
    mTimerComponent->AddTimer(Zoe::FIREBALL_COOLDOWN, [this]()
                              { SetFireballOnCooldown(false); });

    mGame->GetAudio()->PlaySound("fireball.wav");
}

void Zoe::TriggerVentania()
{
    if (mIsVentaniaOnCooldown)
        return;

    mRigidBodyComponent->SetVelocity(Vector2(0.f, 0.f));

    Vector2 ventaniaDir = mInputMovementDir.LengthSq() >= 0.f ? mInputMovementDir : Vector2(0.f, 1.f);

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

    if (mDamageSoundHandle.IsValid())
    {
        mGame->GetAudio()->StopSound(mDamageSoundHandle);
    }

    mDamageSoundHandle = mGame->GetAudio()->PlaySound("zoeTakeDamage.wav");
}

void Zoe::LockAbilitiesForDuration(float duration)
{
    mAbilitiesLocked = true;
    mTimerComponent->AddTimer(duration, [this]()
                              { mAbilitiesLocked = false; });
}

void Zoe::TakeSpikeHit(const Vector2 &SpikeBaseCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - SpikeBaseCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Zoe::SPIKE_KNOCKBACK_FORCE);
}

void Zoe::TakeSpearHit(const Vector2 &SpearTipCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - SpearTipCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Zoe::SPEAR_KNOCKBACK_FORCE);
}

void Zoe::TakeShurikenHit(const Vector2 &ShurikenCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - ShurikenCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Zoe::SHURIKEN_KNOCKBACK_FORCE);
}