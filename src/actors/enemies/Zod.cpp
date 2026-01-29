#include "Zod.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"
#include "ZodProjectile.h"

Zod::Zod(Game* game, float forwardSpeed, const Vector2& position)
    : Enemy(game, position), mProjectileOnCooldown(false)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 8,
        10, 20,
        ColliderLayer::Enemy);
    
    mColliderComponent->IgnoreLayer(ColliderLayer::Nevasca, IgnoreOption::IgnoreResolution);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Zod/texture.png",
        "../assets/Sprites/Enemies/Zod/texture.json",
        std::bind(&Zod::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);
    
    mAIMovementComponent = new AIMovementComponent(
        this, 
        forwardSpeed, 
        4,
        TypeOfMovement::Walker, 
        3.f, 
        .01f);

    mTimerComponent = new TimerComponent(this);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("waking", 1, 4);
    mDrawComponent->AddAnimation("idle", 5, 6);
    mDrawComponent->AddAnimation("damage", 6, 7);
    mDrawComponent->AddAnimation("moving", 8, 15);
    mDrawComponent->AddAnimation("charging", 16, 19);
    mDrawComponent->AddAnimation("dying", 20, 25);
    mDrawComponent->AddAnimation("freeze", 26, 30, false);

    mDrawComponent->SetAnimation("asleep");

    mBehaviorState = BehaviorState::Asleep;
    SetPosition(position);
}

void Zod::FireProjectile()
{
    if (mProjectileOnCooldown)
        return;

    float speed = mGame->GetConfig()->Get<float>("ZOD.PROJECTILE_SPEED");

    new ZodProjectile(
        mGame,
        GetPosition() + GetProjectileOffset(),
        GetGame()->GetZoe()->GetCenter(),
        speed,
        this);

    mProjectileOnCooldown = true;
    
    float cooldown = mGame->GetConfig()->Get<float>("ZOD.PROJECTILE_COOLDOWN");
    
    mTimerComponent->AddTimer(cooldown, [this]() {
        mProjectileOnCooldown = false;
    });
}

void Zod::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool playerInFov = PlayerOnFov(100.f, 400.f);
    float distanceSQToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();

    float viewDistance = 300.f;

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        if (distanceSQToZoe < 40000.f) mBehaviorState = BehaviorState::Waking;
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

        if (PlayerOnSight(viewDistance) && !mProjectileOnCooldown) {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (playerInFov && 
            mAIMovementComponent->GetMovementState() != MovementState::FollowingPath)
        {
            mAIMovementComponent->SeekPlayer();
        }
        
        break;
    }
    
    case BehaviorState::Charging:
        if (!PlayerOnSight(viewDistance)) mBehaviorState = BehaviorState::Moving;
        break;

    case BehaviorState::TakingDamage:
        break;

    case BehaviorState::Dying:
        break;

    case BehaviorState::Frozen:
        break;

    default:
        mBehaviorState = BehaviorState::Asleep;
        break;
    }
}

void Zod::AnimationEndCallback(std::string animationName)
{
    if (animationName == "waking")
    {
        mBehaviorState = BehaviorState::Moving;
        return;
    }

    if (animationName == "charging")
    {
        FireProjectile();
        mBehaviorState = BehaviorState::Moving;
        return;
    }

    if (animationName == "damage")
    {
        mBehaviorState = BehaviorState::Moving;
        mInvincible = false;
        return;
    }

    if (animationName == "dying") {
        SetState(ActorState::Destroy);
    }
}

void Zod::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        mDrawComponent->SetAnimation("asleep");
        break;
    case BehaviorState::Waking:
        mDrawComponent->SetAnimation("waking");
        mDrawComponent->SetAnimFPS(3.f);
        break;
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(6.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(8.f);
        break;
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("damage");
        mDrawComponent->SetAnimFPS(9.f);
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("dying");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    case BehaviorState::Frozen:
        mDrawComponent->SetAnimation("freeze");
        mDrawComponent->SetAnimFPS(6.f);
        break;
    default:
        mDrawComponent->SetAnimation("asleep");
        break;
    }
}

void Zod::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    Enemy::OnVerticalCollision(minOverlap, other);
    
    Actor::OnVerticalCollision(minOverlap, other);
}

void Zod::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    Enemy::OnHorizontalCollision(minOverlap, other);
    
    Actor::OnHorizontalCollision(minOverlap, other);
}