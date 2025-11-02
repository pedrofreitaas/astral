#include "Zod.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Zod::Zod(Game* game, float forwardSpeed, const Vector2& position)
    : Enemy(game, forwardSpeed, position)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 8,
        10, 20,
        ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Zod/texture.png",
        "../assets/Sprites/Enemies/Zod/texture.json",
        std::bind(&Zod::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);
    
    mAIMovementComponent = new AIMovementComponent(
        this, 
        forwardSpeed, 
        3, 
        TypeOfMovement::Walker, 
        10.f, 
        .01f);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("waking", 1, 2);
    mDrawComponent->AddAnimation("idle", 3, 5);
    mDrawComponent->AddAnimation("damage", 6, 7);
    mDrawComponent->AddAnimation("moving", 8, 15);
    mDrawComponent->AddAnimation("charging", 16, 19);
    mDrawComponent->AddAnimation("dying", 20, 25);

    mDrawComponent->SetAnimation("asleep");

    mBehaviorState = BehaviorState::Asleep;
    SetPosition(position);
}

void Zod::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool playerInFov = PlayerOnFov();
    float distanceSQToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();

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

        // movement component takes care of moving.
        mAIMovementComponent->LogState();

        if (PlayerOnSight()) {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (playerInFov && 
            mAIMovementComponent->GetMovementState() != MovementState::FollowingPath)
        {
            SDL_Log("Enemy::ManageState: Player in FOV, seeking...");
            mAIMovementComponent->SeekPlayer();
        }
        
        break;
    }
    case BehaviorState::Charging:
        if (!PlayerOnSight()) mBehaviorState = BehaviorState::Moving;
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
        mBehaviorState = BehaviorState::Moving;
    }
}

void Zod::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mAIMovementComponent->SetFowardSpeed(-mAIMovementComponent->GetFowardSpeed());
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
        mDrawComponent->SetAnimFPS(10.f);
        break;
    default:
        mDrawComponent->SetAnimation("asleep");
        break;
    }
}

void Zod::TakeDamage()
{
}