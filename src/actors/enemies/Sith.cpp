#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Sith::Sith(Game *game, const Vector2 &position)
    : Enemy(game, position, 800.f, 200.f),
      mIsProjectileOnCooldown(false), mIsAttack1OnCooldown(false), mIsAttack2OnCooldown(false),
      mCurrentAttack(Attacks::None), mHasAppliedAttackBoost(false)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f, false);

    mColliderComponent = new AABBColliderComponent(
        this,
        19, 19,
        21, 21,
        ColliderLayer::Enemy);

    mColliderComponent->IgnoreLayer(ColliderLayer::Nevasca, IgnoreOption::IgnoreResolution);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Sith/texture.png",
        "../assets/Sprites/Enemies/Sith/texture.json",
        std::bind(&Sith::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::BelowPlayer));

    mTimerComponent = new TimerComponent(this);

    float forwardSpeed = game->GetConfig()->Get<float>("SITH.SPEED");
    mAIMovementComponent = new AIMovementComponent(
        this,
        forwardSpeed,
        TypeOfMovement::Flier,
        .01f);

    mDrawComponent->AddAnimation("moving", 0, 6);
    mDrawComponent->AddAnimation("damage", 6, 7);
    mDrawComponent->AddAnimation("death", 8, 12);
    mDrawComponent->AddAnimation("attack", 13, 19);
    mDrawComponent->AddAnimation("attack2", 20, 26);
    mDrawComponent->AddAnimation("charging", 27, 32);
    mDrawComponent->AddAnimation("freeze", 33, 37, false);

    mDrawComponent->SetAnimation("moving");

    SetBehaviorState(BehaviorState::Moving);
    SetPosition(position);
}

void Sith::Attack1()
{
    if (mIsAttack1OnCooldown ||
        mBehaviorState == BehaviorState::Attacking ||
        mBehaviorState == BehaviorState::Charging)
        return;

    SetBehaviorState(BehaviorState::Attacking);
    mCurrentAttack = Attacks::Attack1;

    SetAttack1OnCooldown(true);
    float cooldown = mGame->GetConfig()->Get<float>("SITH.ATTACK1_COOLDOWN");

    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetAttack1OnCooldown(false); });

    mHasAppliedAttackBoost = false;
}

void Sith::Attack2()
{
    if (mIsAttack2OnCooldown ||
        mBehaviorState == BehaviorState::Attacking ||
        mBehaviorState == BehaviorState::Charging)
        return;

    SetBehaviorState(BehaviorState::Attacking);
    mCurrentAttack = Attacks::Attack2;

    SetAttack2OnCooldown(true);
    float cooldown = mGame->GetConfig()->Get<float>("SITH.ATTACK2_COOLDOWN");
    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetAttack2OnCooldown(false); });

    mHasAppliedAttackBoost = false;
}

void Sith::FireProjectile()
{
    if (mIsProjectileOnCooldown)
        return;

    Vector2 startPos = GetCenter();
    Vector2 dir = GetGame()->GetZoe()->GetCenter() - GetCenter();
    dir.Normalize();

    new SithProjectile(
        mGame,
        startPos,
        dir,
        this);

    SetProjectileOnCooldown(true);
    float cooldown = mGame->GetConfig()->Get<float>("SITH.PROJECTILE_COOLDOWN");
    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetProjectileOnCooldown(false); });
}

void Sith::ManageState()
{
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
        float speed = mAIMovementComponent->GetSpeed() * .95f;

        if (mRigidBodyComponent->GetVelocity().x > speed)
            SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < -speed)
            SetRotation(Math::Pi);

        if (IsPlayerOnSightThisFrame() && !mIsProjectileOnCooldown)
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        if (HasSeenPlayerRecently())
        {
            mAIMovementComponent->SeekPlayer();
            if (GetDistanceToPlayerSquared() < 1600.f &&
                !mIsAttack1OnCooldown)
            {
                Attack1();
                break;
            }

            if (GetDistanceToPlayerSquared() < 6000.f &&
                !mIsAttack2OnCooldown)
            {
                Attack2();
                break;
            }
        }
        else
            mAIMovementComponent->LoosePlayer();

        break;
    }

    case BehaviorState::Attacking:
    {
        SetInvincibilityOn(); // animation must end to disable colliders (cannot be interrupted by dmg)

        int spriteIdx = mDrawComponent->GetCurrentSprite();

        if (mCurrentAttack == Attacks::Attack1 && spriteIdx >= 2 && !mHasAppliedAttackBoost)
        {
            mHasAppliedAttackBoost = true;
            float extraSpeed = mGame->GetConfig()->Get<float>("SITH.ATTACK1_EXTRA_SPEED");
            mAIMovementComponent->BoostToPlayer(extraSpeed);
        }

        else if (mCurrentAttack == Attacks::Attack2 && spriteIdx >= 4 && !mHasAppliedAttackBoost)
        {
            mHasAppliedAttackBoost = true;
            float extraSpeed = mGame->GetConfig()->Get<float>("SITH.ATTACK2_EXTRA_SPEED");
            mAIMovementComponent->BoostToPlayer(extraSpeed);
        }

        break;
    }

    case BehaviorState::Charging:
        if (!PlayerOnSight())
            SetBehaviorState(BehaviorState::Moving);
        break;
    case BehaviorState::TakingDamage:
        break;
    case BehaviorState::Dying:
        break;
    case BehaviorState::Frozen:
        break;
    default:
        SetBehaviorState(BehaviorState::Asleep);
        break;
    }
}

void Sith::AnimationEndCallback(std::string animationName)
{
    if (animationName == "charging")
    {
        FireProjectile();
        SetBehaviorState(BehaviorState::Moving);
    }

    else if (animationName == "attack")
    {
        SetBehaviorState(BehaviorState::Moving);
        mCurrentAttack = Attacks::None;
        SetInvincibilityOff();
    }

    else if (animationName == "attack2")
    {
        SetBehaviorState(BehaviorState::Moving);
        mCurrentAttack = Attacks::None;
        SetInvincibilityOff();
    }

    else if (animationName == "death")
    {
        SetState(ActorState::Destroy);
        mRigidBodyComponent->SetEnabled(false);
        mColliderComponent->SetEnabled(false);
    }

    else if (animationName == "damage")
    {
        SetBehaviorState(BehaviorState::Moving);
        SetInvincibilityOff();
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
        if (mCurrentAttack == Attacks::Attack1)
        {
            mDrawComponent->SetAnimation("attack");

            if (mDrawComponent->GetCurrentSprite() <= 2)
                mDrawComponent->SetAnimFPS(7.f);
            else
                mDrawComponent->SetAnimFPS(8.f);
        }
        else if (mCurrentAttack == Attacks::Attack2)
        {
            mDrawComponent->SetAnimation("attack2");

            if (mDrawComponent->GetCurrentSprite() <= 4)
                mDrawComponent->SetAnimFPS(7.f);
            else
                mDrawComponent->SetAnimFPS(8.f);
        }
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("death");
        mDrawComponent->SetAnimFPS(5.f);
        break;
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("damage");
        mDrawComponent->SetAnimFPS(9.f);
        break;
    case BehaviorState::Frozen:
        mDrawComponent->SetAnimation("freeze");
        mDrawComponent->SetAnimFPS(6.f);
        break;
    default:
        mDrawComponent->SetAnimation("moving");
        break;
    }
}

void Sith::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    Enemy::OnVerticalCollision(minOverlap, other);

    Actor::OnVerticalCollision(minOverlap, other);
}

void Sith::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    Enemy::OnHorizontalCollision(minOverlap, other);

    Actor::OnHorizontalCollision(minOverlap, other);
}