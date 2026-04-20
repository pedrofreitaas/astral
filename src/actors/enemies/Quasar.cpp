#include "Quasar.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Quasar::Quasar(Game *game, const Vector2 &center)
    : Enemy(game, center), mAppliedImpulseInAttack(false), mAttackTimerHandle(nullptr)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this,
        33, 34,
        23, 29,
        ColliderLayer::Quasar);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Quasar/texture.png",
        "../assets/Sprites/Enemies/Quasar/texture.json",
        std::bind(&Quasar::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);

    mTimerComponent = new TimerComponent(this);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        350.f, 
        0,
        TypeOfMovement::Walker, 
        3.f, 
        .01f);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("idle", 1, 8);
    mDrawComponent->AddAnimation("walk", 9, 18);
    mDrawComponent->AddAnimation("hit", 19, 22);
    mDrawComponent->AddAnimation("die", 23, 34);
    mDrawComponent->AddAnimation("attack", 35, 45);

    mDrawComponent->SetAnimation("asleep");

    mBehaviorState = BehaviorState::Asleep;

    SetPosition(center - GetHalfSize());
}

void Quasar::ManageState()
{
    Zoe* zoe = mGame->GetZoe();
    
    if (!zoe)
        return;

    Vector2 toZoe = zoe->GetCenter() - GetCenter();
    float distanceToZoeSQ = toZoe.LengthSq();

    switch (mBehaviorState)
    {
        case BehaviorState::Asleep:
            if (distanceToZoeSQ <= 45000.f)
            {
                mBehaviorState = BehaviorState::Moving;
            }

            break;

        case BehaviorState::Idle:
            break;

        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetVelocity().x > 0.f)
                SetRotation(0.f);
            else if (mRigidBodyComponent->GetVelocity().x < 0.f)
                SetRotation(Math::Pi);

            if (
                PlayerOnSight(60.f) && 
                (mAttackTimerHandle == nullptr || 
                 mTimerComponent->checkTimerRemaining(mAttackTimerHandle) <= 0.f)
            ) {
                mAppliedImpulseInAttack = false;
                mBehaviorState = BehaviorState::Attacking;
                
                mAttackTimerHandle = mTimerComponent->AddTimer(
                    mGame->GetConfig()->Get<float>("QUASAR.SMASH_COOLDOWN"), 
                    nullptr
                );

                break;
            }

            break;

        case BehaviorState::TakingDamage:
            break;
        
        case BehaviorState::Dying:
            break;

        case BehaviorState::Attacking: {
            if (mAppliedImpulseInAttack)
                break;
            
            if (mDrawComponent->GetCurrentSprite() < 4)
                break;
            
            float forward = GetForward().x;
            mRigidBodyComponent->ApplyImpulse(Vector2(forward * 220.f, 0.f));
            mAppliedImpulseInAttack = true;
            
            break;
        }

        default:
            mBehaviorState = BehaviorState::Asleep;
            break;
    }
}

void Quasar::AnimationEndCallback(std::string animationName)
{
    if (animationName == "attack") {
        mBehaviorState = BehaviorState::Moving;
        return;
    }

    if (animationName == "hit")
    {
        mBehaviorState = BehaviorState::Moving;
        SetInvincibilityOff();
        return;
    }

    if (animationName == "die") {
        SetState(ActorState::Destroy);
        return;
    }
}

void Quasar::ManageAnimations()
{
    switch (mBehaviorState)
    {
        case BehaviorState::Asleep:
            mDrawComponent->SetAnimation("asleep");
            mDrawComponent->SetAnimFPS(1.f);
            break;

        case BehaviorState::Idle:
            mDrawComponent->SetAnimation("idle");
            mDrawComponent->SetAnimFPS(5.f);
            break;

        case BehaviorState::Moving:
            mDrawComponent->SetAnimation("walk");
            mDrawComponent->SetAnimFPS(8.f);
            break;

        case BehaviorState::TakingDamage:
            mDrawComponent->SetAnimation("hit");
            mDrawComponent->SetAnimFPS(6.f);
            break;

        case BehaviorState::Dying:
            mDrawComponent->SetAnimation("die");
            mDrawComponent->SetAnimFPS(14.f);
            break;

        case BehaviorState::Attacking:
            mDrawComponent->SetAnimation("attack");
            mDrawComponent->SetAnimFPS(7.f);
            break;

        default:
            mDrawComponent->SetAnimation("asleep");
            break;
    }
}

void Quasar::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    // Enemy::OnVerticalCollision(minOverlap, other);
    // Actor::OnVerticalCollision(minOverlap, other);
}

void Quasar::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    // Enemy::OnHorizontalCollision(minOverlap, other);
    // Actor::OnHorizontalCollision(minOverlap, other);
}