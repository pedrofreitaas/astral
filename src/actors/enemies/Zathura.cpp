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
    mBlockedPlayerSoundHandle(SoundHandle::Invalid),
    mRockAttackTimerHandle(nullptr), mIsWaitingToThrowRocks(false),
    mAttack1CooldownTimer(nullptr), mAttack2CooldownTimer(nullptr), 
    mAttack3CooldownTimer(nullptr), mSpawnedAttackCollider(false)
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
    mDrawComponent->AddAnimation("attack1", 28, 38);
    mDrawComponent->AddAnimation("attack2", 39, 46); // punch
    mDrawComponent->AddAnimation("attack3", 47, 53);
    mDrawComponent->AddAnimation("frozen", 54, 58, false);
    mDrawComponent->AddAnimation("vanish", 59, 63, false);
    mDrawComponent->AddAnimation("appear", {63,62,61,60,59}, false);

    mDrawComponent->SetAnimation("walk");

    SetBehaviorState(BehaviorState::Idle);

    SetPosition(center - GetHalfSize());

    SetLifes(game->GetConfig()->Get<int>("Zathura.HEALTH"));

    mGame->SetZathura(this);
}

void Zathura::ManageState()
{
    switch (mBehaviorState)
    {
        case BehaviorState::Idle: {
            if (GetIsWaitingToThrowRocks())
                break;

            SetBehaviorState(BehaviorState::Moving);

            break;
        }

        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetVelocity().x > 0.f)
                SetRotation(0.f);
            else if (mRigidBodyComponent->GetVelocity().x < 0.f)
                SetRotation(Math::Pi);
            
            if (!mRigidBodyComponent->GetOnGround())
                break;

            if (
                (mRockAttackTimerHandle == nullptr ||
                 mTimerComponent->checkTimerRemaining(mRockAttackTimerHandle) <= 0.f)
            )
            {
                SetBehaviorState(BehaviorState::Vanishing);
                break;
            }
            
            if (PlayerOnFov()) mAIMovementComponent->SeekPlayer();
            else mAIMovementComponent->LoosePlayer();

            if (!IsPlayerOnSightThisFrame())
                break;

            if (
                GetDistanceToPlayerSquared() <= 10000 &&
                (mAttack1CooldownTimer == nullptr ||
                 mTimerComponent->checkTimerRemaining(mAttack1CooldownTimer) <= 0.f)
            )
            {
                SetBehaviorState(BehaviorState::Attacking);
                mCurrentAttack = ZathuraAttacks::Attack1;
                mSpawnedAttackCollider = false;
                mAttack1CooldownTimer = mTimerComponent->AddTimer(
                    mGame->GetConfig()->Get<float>("ZATHURA.ATTACK1_COOLDOWN"), 
                    nullptr
                );
                break;
            }

            if (
                GetDistanceToPlayerSquared() <= 12100 &&
                (mAttack2CooldownTimer == nullptr ||
                 mTimerComponent->checkTimerRemaining(mAttack2CooldownTimer) <= 0.f)
            )
            {
                SetBehaviorState(BehaviorState::Attacking);
                mCurrentAttack = ZathuraAttacks::Attack2;
                mSpawnedAttackCollider = false;
                mAttack2CooldownTimer = mTimerComponent->AddTimer(
                    mGame->GetConfig()->Get<float>("ZATHURA.ATTACK2_COOLDOWN"), 
                    nullptr
                );
                break;
            }

            if (
                GetDistanceToPlayerSquared() <= 8100 &&
                (mAttack3CooldownTimer == nullptr ||
                 mTimerComponent->checkTimerRemaining(mAttack3CooldownTimer) <= 0.f)
            )
            {
                SetBehaviorState(BehaviorState::Attacking);
                mCurrentAttack = ZathuraAttacks::Attack3;
                mSpawnedAttackCollider = false;
                mAttack3CooldownTimer = mTimerComponent->AddTimer(
                    mGame->GetConfig()->Get<float>("ZATHURA.ATTACK3_COOLDOWN"), 
                    nullptr
                );
                break;
            }

            break;

        case BehaviorState::Vanishing:
            break;

        case BehaviorState::Appearing:
            break;

        case BehaviorState::TakingDamage:
            break;
        
        case BehaviorState::Dying:
            break;

        case BehaviorState::Attacking: {
            int currentSprite = mDrawComponent->GetCurrentSprite();
            
            switch (mCurrentAttack)
            {
                case ZathuraAttacks::Attack1: {
                    if (currentSprite >= 7 && !mSpawnedAttackCollider)
                    {
                        mSpawnedAttackCollider = true;
                        
                        Vector2 offset = GetForward().x == 1 ? Vector2(131, 107) : Vector2(12, 107);

                        new Collider(
                            mGame,
                            this,
                            Vector2(GetPosition() + offset),
                            Vector2(56, 24),
                            nullptr, 
                            DismissOn::Both,
                            ColliderLayer::ZathuraAttack1,
                            {},
                            .5f,
                            nullptr,
                            false,
                            nullptr
                        );
                    }

                    break;
                }

                case ZathuraAttacks::Attack2: {
                    if (currentSprite >= 5 && !mSpawnedAttackCollider)
                    {
                        mSpawnedAttackCollider = true;

                        Vector2 offset = GetForward().x == 1 ? Vector2(127, 82) : Vector2(23, 82);

                        new Collider(
                            mGame,
                            this,
                            Vector2(GetPosition() + offset),
                            Vector2(49, 49),
                            nullptr, 
                            DismissOn::Both,
                            ColliderLayer::ZathuraAttack2,
                            {},
                            .75f,
                            nullptr,
                            false,
                            nullptr
                        );
                    }

                    break;
                }

                case ZathuraAttacks::Attack3: {
                    if (currentSprite >= 4 && !mSpawnedAttackCollider)
                    {
                        mSpawnedAttackCollider = true;

                        Vector2 offset = GetForward().x == 1 ? Vector2(128, 96) : Vector2(39, 96);

                        new Collider(
                            mGame,
                            this,
                            Vector2(GetPosition() + offset),
                            Vector2(33, 36),
                            nullptr, 
                            DismissOn::Both,
                            ColliderLayer::ZathuraAttack3,
                            {},
                            .75f,
                            nullptr,
                            false,
                            nullptr
                        );
                    }
                    
                    break;
                }

                case ZathuraAttacks::Rocks: {
                    Rock::SpawnRocks(mGame, this);
                    
                    mCurrentAttack = ZathuraAttacks::None;
                    
                    SetBehaviorState(BehaviorState::Appearing);
                    
                    mRockAttackTimerHandle = mTimerComponent->AddTimer(
                        mGame->GetConfig()->Get<float>("ZATHURA.ROCKS_ATTACK_COOLDOWN"), 
                        nullptr
                    );
                }
                default:
                    SDL_Log("Invalid attack animation name!");
            }

            break;
        }

        case BehaviorState::Frozen:
            break;

        default:
            SetBehaviorState(BehaviorState::Asleep);
            break;
    }
}

void Zathura::AnimationEndCallback(std::string animationName)
{    
    if (animationName == "appear")
    {
        SetBehaviorState(BehaviorState::Idle);
        return;
    }

    if (animationName == "vanish")
    {
        int getWindowCenter = mGame->GetCameraPos().x + mGame->GetWindowWidth() * 0.5f;

        SetCenter(Vector2(
            getWindowCenter,
            GetCenter().y
        ));

        SetBehaviorState(BehaviorState::Attacking);
        mCurrentAttack = ZathuraAttacks::Rocks;

        return;
    }
    
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

        case BehaviorState::Attacking: {
            switch (mCurrentAttack)
            {
                case ZathuraAttacks::Attack1:
                    mDrawComponent->SetAnimation("attack1");
                    mDrawComponent->SetAnimFPS(7.f);
                    break;
                case ZathuraAttacks::Attack2:
                    mDrawComponent->SetAnimation("attack2");
                    mDrawComponent->SetAnimFPS(7.f);
                    break;
                case ZathuraAttacks::Attack3:
                    mDrawComponent->SetAnimation("attack3");
                    mDrawComponent->SetAnimFPS(7.f);
                    break;
                case ZathuraAttacks::Rocks:
                    break;
                default:
                    SDL_Log("Invalid attack animation name!");
                    break;
            }
        
            break;
        }

        case BehaviorState::Vanishing:
            mDrawComponent->SetAnimation("vanish");
            mDrawComponent->SetAnimFPS(7.f);
            break;

        case BehaviorState::Appearing:
            mDrawComponent->SetAnimation("appear");
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
        mBlockedPlayerSoundHandle = mGame->GetAudio()->PlaySound("playerHitBlock.wav");
        return;
    }

    mGame->GetAudio()->PlaySound("playerHitBlock.wav");
}