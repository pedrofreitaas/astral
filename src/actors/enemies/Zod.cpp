#include "Zod.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"
#include "ZodProjectile.h"

Zod::Zod(Game* game, const Vector2& position)
    : Enemy(game, position, 400.f, 80.f), mProjectileOnCooldown(false)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 8,
        10, 20,
        ColliderLayer::Enemy);
    
    mColliderComponent->IgnoreLayers({
        ColliderLayer::Nevasca,
        ColliderLayer::Quasar,
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile,
        ColliderLayer::Torch
    }, IgnoreOption::IgnoreResolution);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Zod/texture.png",
        "../assets/Sprites/Enemies/Zod/texture.json",
        std::bind(&Zod::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::BelowPlayer));
    
    mTimerComponent = new TimerComponent(this);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        1200.f, 
        TypeOfMovement::Walker);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("waking", 1, 4);
    mDrawComponent->AddAnimation("idle", 5, 6);
    mDrawComponent->AddAnimation("damage", 6, 7);
    mDrawComponent->AddAnimation("moving", 8, 15);
    mDrawComponent->AddAnimation("charging", 16, 19);
    mDrawComponent->AddAnimation("dying", 20, 25);
    mDrawComponent->AddAnimation("freeze", 26, 30, false);

    mDrawComponent->SetAnimation("asleep");

    SetBehaviorState(BehaviorState::Asleep);
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

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        if (GetDistanceToPlayerSquared() < 50000.f) SetBehaviorState(BehaviorState::Waking);
        break;
    case BehaviorState::Waking:
        break;
    case BehaviorState::Idle:
        break;
    case BehaviorState::Moving:
    {
        if (mRigidBodyComponent->GetVelocity().x > 0.f) SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < 0.f) SetRotation(Math::Pi);

        if (IsPlayerOnSightThisFrame() && !mProjectileOnCooldown) 
        {
            SetBehaviorState(BehaviorState::Charging);
            break;
        }

        if (PlayerOnFov()) mAIMovementComponent->SeekPlayer();
        else mAIMovementComponent->LoosePlayer();
        
        break;
    }
    
    case BehaviorState::Charging: {
        if (!IsPlayerOnSightThisFrame()) SetBehaviorState(BehaviorState::Moving);

        int currentAnimationSprite = mDrawComponent->GetCurrentSprite();

        if (
            currentAnimationSprite > 2 && 
            !GetGame()->GetConfig()->Get<bool>("ZOE.SHOWN_DODGE_CUTSCENE")
        ) {
            PlayDodgeCutscene();
        }

        break;
    }

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

void Zod::AnimationEndCallback(std::string animationName)
{
    if (animationName == "waking")
    {
        SetBehaviorState(BehaviorState::Moving);
        return;
    }

    if (animationName == "charging")
    {
        FireProjectile();
        SetBehaviorState(BehaviorState::Moving);
        return;
    }

    if (animationName == "damage")
    {
        SetBehaviorState(BehaviorState::Moving);
        SetInvincibilityOff();
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
        mDrawComponent->SetAnimFPS(4.5f);
        break;
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
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
            mDrawComponent->SetAnimation("moving");
            mDrawComponent->SetAnimFPS(8.f);
        }
        break;
    }
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(8.f);
        break;
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("damage");
        mDrawComponent->SetAnimFPS(5.f);
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("dying");
        mDrawComponent->SetAnimFPS(9.f);
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

void Zod::PlayDodgeCutscene() {
    GetGame()->GetConfig()->Update("ZOE.SHOWN_DODGE_CUTSCENE", true, false);
    std::vector<std::unique_ptr<Step>> steps;

    std::vector<std::string> dialogue = {
        "Esse tiro esta vindo na minha direcao."
    };

    steps.push_back(std::make_unique<DialogueStep>(mGame, "Zoe", dialogue));

    steps.push_back(std::make_unique<FreezePhysicsStep>(mGame));
    
    steps.push_back(std::make_unique<SpawnJoystickButtonStep>(mGame, Button::B));

    steps.push_back(std::make_unique<DodgeStep>(mGame));

    steps.push_back(std::make_unique<UnfreezePhysicsStep>(mGame));

    mGame->AddCutscene("dodgeCutscene",std::move(steps));

    mGame->StartCutscene("dodgeCutscene");
}