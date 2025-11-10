#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

SithProjectile::SithProjectile(
    class Game* game, Vector2 position, 
    Vector2 target, float speed
): Projectile(game, position, target, speed)
{
    const std::string spriteSheetPath = "../assets/Sprites/Enemies/Sith/Projectile/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Enemies/Sith/Projectile/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 18,
        14, 12,
        ColliderLayer::Projectile);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&SithProjectile::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 2);
    mDrawAnimatedComponent->AddAnimation("dying", 3, 7);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - mDrawAnimatedComponent->GetHalfSpriteSize());
    mDirection = target - GetCenter();
    mDirection.Normalize();
}

void SithProjectile::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying") {
        SetState(ActorState::Destroy);
    }
}

void SithProjectile::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying) {
        mDrawAnimatedComponent->SetAnimation("dying");
    }
    else if (mBehaviorState == BehaviorState::Moving) {
        mDrawAnimatedComponent->SetAnimation("flying");
    }
}

// Sith

Sith::Sith(Game* game, float forwardSpeed, const Vector2& position)
    : Enemy(game, forwardSpeed, position), 
    mIsProjectileOnCooldown(false), mIsAttack1OnCooldown(false), mIsAttack2OnCooldown(false),
    mCurrentAttack(Attacks::None)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f, false);
    mColliderComponent = new AABBColliderComponent(
        this,
        19, 19,
        21, 21,
        ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Sith/texture.png",
        "../assets/Sprites/Enemies/Sith/texture.json",
        std::bind(&Sith::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);

    mTimerComponent = new TimerComponent(this);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        forwardSpeed, 
        -1, 
        TypeOfMovement::Flier,
        5.f, 
        .01f);

    mDrawComponent->AddAnimation("moving", 0, 7);
    mDrawComponent->AddAnimation("damage", 8, 11);
    mDrawComponent->AddAnimation("death", 12, 15);
    mDrawComponent->AddAnimation("attack", 16, 23);
    mDrawComponent->AddAnimation("attack2", 24, 31);
    mDrawComponent->AddAnimation("charging", 32, 37);

    mDrawComponent->SetAnimation("moving");

    mBehaviorState = BehaviorState::Moving;
    SetPosition(position);
}

void Sith::Attack1()
{
    if (mIsAttack1OnCooldown || 
        mBehaviorState == BehaviorState::Attacking || 
        mBehaviorState == BehaviorState::Charging)
        return;

    mBehaviorState = BehaviorState::Attacking;
    mCurrentAttack = Attacks::Attack1;

    SetAttack1OnCooldown(true);
    mTimerComponent->AddTimer(Sith::ATTACK1_COOLDOWN, [this]() {
        SetAttack1OnCooldown(false);
    });
}

void Sith::Attack2()
{
    if (mIsAttack2OnCooldown ||
        mBehaviorState == BehaviorState::Attacking ||
        mBehaviorState == BehaviorState::Charging)
        return;

    mBehaviorState = BehaviorState::Attacking;
    mCurrentAttack = Attacks::Attack2;

    SetAttack2OnCooldown(true);
    mTimerComponent->AddTimer(Sith::ATTACK2_COOLDOWN, [this]() {
        SetAttack2OnCooldown(false);
    });
}

void Sith::FireProjectile()
{
    if (mIsProjectileOnCooldown)
        return;
    
    auto projectile = new SithProjectile(
        mGame,
        GetPosition() + GetProjectileOffset(),
        GetGame()->GetZoe()->GetCenter(),
        Sith::PROJECTILE_SPEED);

    SetProjectileOnCooldown(true);
    mTimerComponent->AddTimer(Sith::PROJECTICLE_COOLDOWN, [this]() {
        SetProjectileOnCooldown(false);
    });
}

void Sith::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool playerInFov = PlayerOnFov();
    float distanceSQToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();
    float sightDistance = 400.f;

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        break;
    case BehaviorState::Waking:
        break;
    case BehaviorState::Idle:
        break;
    case BehaviorState::Moving:
    {
        if (mRigidBodyComponent->GetVelocity().x > 0.f)
            SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < 0.f)
            SetRotation(Math::Pi);

        if (PlayerOnSight(sightDistance) && !mIsProjectileOnCooldown) {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (distanceSQToZoe < 2500.f && 
            !mIsAttack1OnCooldown &&
            mAIMovementComponent->CrazyDecision(2.f)
        ) {
            Attack1();
            break;
        }

        if (distanceSQToZoe < 12000.f &&
            !mIsAttack2OnCooldown &&
            mAIMovementComponent->CrazyDecision(3.f)
        ) {
            Attack2();
            break;
        }

        if (playerInFov && 
            mAIMovementComponent->GetMovementState() != MovementState::FollowingPath)
        {
            mAIMovementComponent->SeekPlayer();
        }
        
        break;
    }
    case BehaviorState::Attacking:
        if (mCurrentAttack == Attacks::Attack2) {
            mAIMovementComponent->BoostToPlayer(Sith::ATTACK2_EXTRA_SPEED);
        }
        break;
    case BehaviorState::Charging:
        if (!PlayerOnSight(sightDistance)) mBehaviorState = BehaviorState::Moving;
        break;
    case BehaviorState::TakingDamage:
        break;
    case BehaviorState::Dying:
        break;
    default:
        mBehaviorState = BehaviorState::Asleep;
        break;
    }
}

void Sith::AnimationEndCallback(std::string animationName)
{
    if (animationName == "charging") {
        FireProjectile();
        mBehaviorState = BehaviorState::Moving;
    }

    else if (animationName == "attack" || animationName == "attack2") {
        mBehaviorState = BehaviorState::Moving;
        mCurrentAttack = Attacks::None;
    }

    else if (animationName == "death") {
        SetState(ActorState::Destroy);
    }

    else if (animationName == "damage")
    {
        mBehaviorState = BehaviorState::Moving;
        mInvincible = false;
    }
}

void Sith::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(8.f);
        break;
    case BehaviorState::Attacking:
        if (mCurrentAttack == Attacks::Attack1) {
            mDrawComponent->SetAnimation("attack");
            mDrawComponent->SetAnimFPS(12.f);
        }
        else if (mCurrentAttack == Attacks::Attack2) {
            mDrawComponent->SetAnimation("attack2");
            mDrawComponent->SetAnimFPS(12.f);
        }
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("death");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("damage");
        mDrawComponent->SetAnimFPS(9.f);
        break;
    default:
        mDrawComponent->SetAnimation("moving");
        break;
    }
}

void Sith::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player && minOverlap < 0.f)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        TakeDamage();
        return;
    }
}

void Sith::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        TakeDamage();
        return;
    }
}