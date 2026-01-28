#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Sith::Sith(Game *game, const Vector2 &position)
    : Enemy(game, position),
      mIsProjectileOnCooldown(false), mIsAttack1OnCooldown(false), mIsAttack2OnCooldown(false),
      mCurrentAttack(Attacks::None), mAttack1Collider(nullptr), mAttack2Collider(nullptr)
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

    float forwardSpeed = game->GetConfig()->Get<float>("SITH.SPEED");
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

    mAttack2Collider = new Collider(
        mGame,
        this,
        GetCenter() - Vector2(15.f, 15.f),
        Vector2(20, 20),
        [this](bool collided, float minOverlap, AABBColliderComponent *other)
        {
            if (collided && other->GetOwner() == GetGame()->GetZoe())
            {
                mAttack2Collider->SetEnabled(false);
            }
        },
        DismissOn::None,
        ColliderLayer::SithAttack2,
        {ColliderLayer::Enemy},
        -1.f,
        nullptr,
        false);
    mAttack2Collider->SetEnabled(false);
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
    float cooldown = mGame->GetConfig()->Get<float>("SITH.ATTACK1_COOLDOWN");

    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetAttack1OnCooldown(false); });
}

void Sith::Attack2()
{
    if (mIsAttack2OnCooldown ||
        mBehaviorState == BehaviorState::Attacking ||
        mBehaviorState == BehaviorState::Charging)
        return;

    mBehaviorState = BehaviorState::Attacking;
    mCurrentAttack = Attacks::Attack2;
    mAttack2Collider->SetEnabled(true);

    SetAttack2OnCooldown(true);
    float cooldown = mGame->GetConfig()->Get<float>("SITH.ATTACK2_COOLDOWN");
    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetAttack2OnCooldown(false); });
}

void Sith::FireProjectile()
{
    if (mIsProjectileOnCooldown)
        return;

    auto projectile = new SithProjectile(
        mGame,
        GetPosition() + GetProjectileOffset(),
        GetGame()->GetZoe()->GetCenter(),
        this);

    SetProjectileOnCooldown(true);
    float cooldown = mGame->GetConfig()->Get<float>("SITH.PROJECTILE_COOLDOWN");
    mTimerComponent->AddTimer(cooldown, [this]()
                              { SetProjectileOnCooldown(false); });
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

        if (PlayerOnSight(sightDistance) && !mIsProjectileOnCooldown)
        {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (distanceSQToZoe < 2500.f &&
            !mIsAttack1OnCooldown &&
            mAIMovementComponent->CrazyDecision(2.f))
        {
            Attack1();
            break;
        }

        if (distanceSQToZoe < 12000.f &&
            !mIsAttack2OnCooldown &&
            mAIMovementComponent->CrazyDecision(3.f))
        {
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
        mInvincible = true; // animation must end to disable colliders (cannot be interrupted by dmg)

        if (mCurrentAttack == Attacks::Attack1)
        {
            float extraSpeed = mGame->GetConfig()->Get<float>("SITH.ATTACK1_EXTRA_SPEED");
            mAIMovementComponent->BoostToPlayer(extraSpeed);
        }

        else if (mCurrentAttack == Attacks::Attack2)
        {
            float extraSpeed = mGame->GetConfig()->Get<float>("SITH.ATTACK2_EXTRA_SPEED");
            mAIMovementComponent->BoostToPlayer(extraSpeed);
            Vector2 attackColliderPosition = GetCenter() - Vector2(15.f, 15.f);
            mAttack2Collider->SetPosition(attackColliderPosition);
        }
        break;
    case BehaviorState::Charging:
        if (!PlayerOnSight(sightDistance))
            mBehaviorState = BehaviorState::Moving;
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
    if (animationName == "charging")
    {
        FireProjectile();
        mBehaviorState = BehaviorState::Moving;
    }

    else if (animationName == "attack")
    {
        mBehaviorState = BehaviorState::Moving;
        mCurrentAttack = Attacks::None;
        if (mAttack1Collider)
            mAttack1Collider->Dismiss();
        mAttack1Collider = nullptr;
        mInvincible = false;
    }

    else if (animationName == "attack2")
    {
        mBehaviorState = BehaviorState::Moving;
        mCurrentAttack = Attacks::None;
        mAttack2Collider->SetEnabled(false);
        mInvincible = false;
    }

    else if (animationName == "death")
    {
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
        if (mCurrentAttack == Attacks::Attack1)
        {
            mDrawComponent->SetAnimation("attack");
            mDrawComponent->SetAnimFPS(8.f);

            if (mDrawComponent->GetCurrentSprite() >= 5 && mAttack1Collider == nullptr)
            {
                mAttack1Collider = new Collider(
                    mGame,
                    this,
                    GetPosition() + (GetRotation() == 0.f ? Vector2(30.f, 30.f) : Vector2(7.f, 30.f)),
                    Vector2(17.f, 8.f),
                    [this](bool collided, float minOverlap, AABBColliderComponent *other)
                    {
                        if (collided && other->GetOwner() == GetGame()->GetZoe())
                        {
                            mAttack1Collider->SetEnabled(false);
                        }
                    },
                    DismissOn::Both,
                    ColliderLayer::SithAttack1,
                    {ColliderLayer::Enemy},
                    .5f,
                    nullptr,
                    false);
            }
        }
        else if (mCurrentAttack == Attacks::Attack2)
        {
            mDrawComponent->SetAnimation("attack2");
            mDrawComponent->SetAnimFPS(12.f);
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