#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Sith::Sith(Game* game, float forwardSpeed, const Vector2& position)
    : Enemy(game, forwardSpeed, position)
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

    mAIMovementComponent = new AIMovementComponent(this, forwardSpeed, 3, TypeOfMovement::Flier, 10.f, .01f);

    mDrawComponent->AddAnimation("moving", 28, 35);

    mDrawComponent->SetAnimation("moving");

    mBehaviorState = BehaviorState::Moving;
    SetPosition(position);
}

void Sith::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool playerInFov = PlayerOnFov();
    float distanceSQToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();

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

void Sith::AnimationEndCallback(std::string animationName)
{
}

void Sith::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    default:
        mDrawComponent->SetAnimation("moving");
        break;
    }
}

void Sith::TakeDamage()
{
}

void Sith::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mAIMovementComponent->SetFowardSpeed(-mAIMovementComponent->GetFowardSpeed());
    }
}

void Sith::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
    }
}
