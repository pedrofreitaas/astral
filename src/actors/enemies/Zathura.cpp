#include "Zathura.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Zathura::Zathura(Game *game, const Vector2 &center)
    : Enemy(game, center, 300.f), mCurrentAttack(ZathuraAttacks::None),
    mBlockedPlayerSoundHandle(SoundHandle::Invalid)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this,
        92, 84,
        22, 43,
        ColliderLayer::Zathura);

    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::Nevasca
    }, IgnoreOption::IgnoreResolution);
    
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Zathura/texture.png",
        "../assets/Sprites/Enemies/Zathura/texture.json",
        std::bind(&Zathura::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::BelowPlayer));

    mTimerComponent = new TimerComponent(this);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        400.f,
        TypeOfMovement::Walker,
        0.01f);

    mDrawComponent->AddAnimation("idle", 0, 9);
    mDrawComponent->AddAnimation("walk", 10, 17);
    mDrawComponent->AddAnimation("hit", 18, 20);
    mDrawComponent->AddAnimation("die", 21, 27);
    mDrawComponent->AddAnimation("attack1", 28, 37);
    mDrawComponent->AddAnimation("attack2", 38, 46); // punch
    mDrawComponent->AddAnimation("attack3", 47, 53);
    mDrawComponent->AddAnimation("frozen", 54, 58, false);

    mDrawComponent->SetAnimation("idle");

    SetBehaviorState(BehaviorState::Idle);

    SetPosition(center - GetHalfSize());

    SetLifes(game->GetConfig()->Get<int>("Zathura.HEALTH"));

    mGame->SetZathura(this);
}

void Zathura::ManageState()
{
    Zoe* zoe = mGame->GetZoe();
    
    if (!zoe)
        return;

    switch (mBehaviorState)
    {
        case BehaviorState::Idle:
            SetBehaviorState(BehaviorState::Moving);
            break;

        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetVelocity().x > 0.f)
                SetRotation(0.f);
            else if (mRigidBodyComponent->GetVelocity().x < 0.f)
                SetRotation(Math::Pi);
            
            if (!mRigidBodyComponent->GetOnGround())
                break;

            if (PlayerOnFov()) mAIMovementComponent->SeekPlayer();
            else mAIMovementComponent->LoosePlayer();

            break;

        case BehaviorState::TakingDamage:
            break;
        
        case BehaviorState::Dying:
            break;

        case BehaviorState::Attacking:
            break;

        case BehaviorState::Frozen:
            break;

        default:
            SetBehaviorState(BehaviorState::Asleep);
            break;
    }
}

void Zathura::AnimationEndCallback(std::string animationName)
{
    if (animationName == "attack1" || animationName == "attack2" || animationName == "attack3") {
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

void Zathura::ManageAnimations()
{
    switch (mBehaviorState)
    {
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

        case BehaviorState::Frozen:
            mDrawComponent->SetAnimation("frozen");
            mDrawComponent->SetAnimFPS(6.f);
            break;

        default:
            mDrawComponent->SetAnimation("asleep");
            break;
    }
}

void Zathura::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnVerticalCollision(minOverlap, other);
    Actor::OnVerticalCollision(minOverlap, other);
    
    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        Zoe *zoe = mGame->GetZoe();
        zoe->TakeKnockback(
            Vector2(
                zoe->GetCenter().x > GetCenter().x ? 100.f : -100.f, 
                -200.f
            )
        );
        
        PlayBlockedPlayerSound();
    }
}

void Zathura::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnHorizontalCollision(minOverlap, other);
    Actor::OnHorizontalCollision(minOverlap, other);
    
    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        Zoe *zoe = mGame->GetZoe();
        
        zoe->TakeKnockback(
            Vector2(
                zoe->GetCenter().x > GetCenter().x ? 100.f : -100.f, 
                -200.f
            )
        );
        
        PlayBlockedPlayerSound();
    }
}

void Zathura::PlayBlockedPlayerSound() {
    if (mGame->GetAudio()->GetSoundState(mBlockedPlayerSoundHandle) == SoundState::Playing)
    {
        mGame->GetAudio()->StopSound(mBlockedPlayerSoundHandle);
    }

    if (!mBlockedPlayerSoundHandle.IsValid()) 
    {
        mBlockedPlayerSoundHandle = mGame->GetAudio()->PlaySound("ZathuraBlock.wav");
        return;
    }

    mGame->GetAudio()->PlaySound("ZathuraBlock.wav");
}