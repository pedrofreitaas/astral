#include "Quasar.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

Quasar::Quasar(Game *game, float forwardSpeed, const Vector2 &center)
    : Enemy(game, center)
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

    mAIMovementComponent = nullptr;

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->SetAnimation("asleep");

    mBehaviorState = BehaviorState::Asleep;

    SetPosition(center - GetHalfSize());
}

void Quasar::ManageState()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        break;
    default:
        mBehaviorState = BehaviorState::Asleep;
        break;
    }
}

void Quasar::AnimationEndCallback(std::string animationName)
{
}

void Quasar::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        mDrawComponent->SetAnimation("asleep");
        mDrawComponent->SetAnimFPS(1.f);
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