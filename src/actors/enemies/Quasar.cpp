#include "Quasar.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Quasar::Quasar(Game *game, const Vector2 &center)
    : Enemy(game, center, 300.f, 120.f), mAppliedImpulseInAttack(false), mAttackTimerHandle(nullptr),
    mBlockedPlayerSoundHandle(SoundHandle::Invalid)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this,
        33, 34,
        23, 29,
        ColliderLayer::Quasar);

    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::Nevasca,
        ColliderLayer::Quasar,
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile,
        ColliderLayer::Torch
    }, IgnoreOption::IgnoreResolution);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Quasar/texture.png",
        "../assets/Sprites/Enemies/Quasar/texture.json",
        std::bind(&Quasar::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::BelowPlayer));

    mTimerComponent = new TimerComponent(this);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        400.f,
        TypeOfMovement::Walker,
        0.01f);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("idle", 1, 8);
    mDrawComponent->AddAnimation("walk", 9, 19);
    mDrawComponent->AddAnimation("hit", 20, 23);
    mDrawComponent->AddAnimation("die", 24, 35);
    mDrawComponent->AddAnimation("attack", 36, 45);
    mDrawComponent->AddAnimation("frozen", 46, 50, false);

    mDrawComponent->SetAnimation("asleep");

    SetBehaviorState(BehaviorState::Asleep);

    SetPosition(center - GetHalfSize());

    SetLifes(game->GetConfig()->Get<int>("QUASAR.HEALTH"));
}

void Quasar::ManageState()
{
    Zoe* zoe = mGame->GetZoe();
    
    if (!zoe)
        return;

    switch (mBehaviorState)
    {
        case BehaviorState::Asleep:
            if (GetDistanceToPlayerSquared() <= 45000.f)
            {
                SetBehaviorState(BehaviorState::Moving);
            }

            break;

        case BehaviorState::Idle:
            break;

        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetVelocity().x > 0.f)
                SetRotation(0.f);
            else if (mRigidBodyComponent->GetVelocity().x < 0.f)
                SetRotation(Math::Pi);
            
            if (!mRigidBodyComponent->GetOnGround())
                break;

            if (
                IsPlayerOnSightThisFrame() &&
                (mAttackTimerHandle == nullptr ||
                 mTimerComponent->checkTimerRemaining(mAttackTimerHandle) <= 0.f)
            )
            {
                mIsCloseAttack = GetLastSeenPlayerDistanceSquared() <= 1600.f;
                mAppliedImpulseInAttack = false;
                SetBehaviorState(BehaviorState::Attacking);
                
                mAttackTimerHandle = mTimerComponent->AddTimer(
                    mGame->GetConfig()->Get<float>("QUASAR.SMASH_COOLDOWN"), 
                    nullptr
                );

                break;
            }

            if (PlayerOnFov()) mAIMovementComponent->SeekPlayer();
            else mAIMovementComponent->LoosePlayer();

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
            
            if (mIsCloseAttack) mRigidBodyComponent->ApplyImpulse(Vector2(forward * 220.f, 0.f));
            else mRigidBodyComponent->ApplyImpulse(Vector2(forward * 400.f, -180.f));

            mAppliedImpulseInAttack = true;
            
            break;
        }

        case BehaviorState::Frozen:
            break;

        default:
            SetBehaviorState(BehaviorState::Asleep);
            break;
    }
}

void Quasar::AnimationEndCallback(std::string animationName)
{
    if (animationName == "attack") {
        SetBehaviorState(BehaviorState::Moving);
        return;
    }

    if (animationName == "hit")
    {
        SetBehaviorState(BehaviorState::Moving);
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

        case BehaviorState::Moving: {
            RigidBodyComponent* rb = GetComponent<RigidBodyComponent>();

            Vector2 velocity = rb->GetVelocity();

            if (velocity.LengthSq() <= 0.01f)
            {
                mDrawComponent->SetAnimation("idle");
                mDrawComponent->SetAnimFPS(5.f);
            }
            else
            {
                mDrawComponent->SetAnimation("walk");
                mDrawComponent->SetAnimFPS(8.f);
            }
            break;
        }

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

        case BehaviorState::Frozen:
            mDrawComponent->SetAnimation("frozen");
            mDrawComponent->SetAnimFPS(6.f);
            break;

        default:
            mDrawComponent->SetAnimation("asleep");
            break;
    }
}

void Quasar::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnVerticalCollision(minOverlap, other);
    Actor::OnVerticalCollision(minOverlap, other);
    
    if (other->GetLayer() == ColliderLayer::Player && minOverlap > 0)
    {
        TakeKnockback(Vector2(-1.f, -1.f) * mGame->GetConfig()->Get<float>("QUASAR.SPIKE_KNOCKBACK_FORCE"));
        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        Zoe *zoe = mGame->GetZoe();

        if (zoe->IsChargedPlayerAttack()) {
            TakeKnockback(Vector2(0.f, -520.f));
        } else {
            zoe->TakeKnockback(
                Vector2(
                    zoe->GetCenter().x > GetCenter().x ? 100.f : -100.f, 
                    -200.f
                )
            );
            
            PlayBlockedPlayerSound();
        }
    }
}

void Quasar::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnHorizontalCollision(minOverlap, other);
    Actor::OnHorizontalCollision(minOverlap, other);
    
    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        Zoe *zoe = mGame->GetZoe();
        
        if (zoe->IsChargedPlayerAttack()) {
            TakeKnockback(Vector2(0.f, -520.f));
        } else {
            zoe->TakeKnockback(
                Vector2(
                    zoe->GetCenter().x > GetCenter().x ? 100.f : -100.f, 
                    -200.f
                )
            );
            
            PlayBlockedPlayerSound();
        }
    }
}

void Quasar::PlayBlockedPlayerSound() {
    if (mGame->GetAudio()->GetSoundState(mBlockedPlayerSoundHandle) == SoundState::Playing)
    {
        mGame->GetAudio()->StopSound(mBlockedPlayerSoundHandle);
    }

    if (!mBlockedPlayerSoundHandle.IsValid()) 
    {
        mBlockedPlayerSoundHandle = mGame->GetAudio()->PlaySound("playerHitBlock.wav");
        return;
    }

    mGame->GetAudio()->PlaySound("playerHitBlock.wav");
}