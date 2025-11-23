#include "Shuriken.h"

Shuriken::Shuriken(Game *game, const Vector2 &position)
    : Actor(game, 1.f)
{
    mTimerComponent = new TimerComponent(this);

    mColliderComponent = new AABBColliderComponent(
        this, 3, 3, 26, 26, ColliderLayer::Shuriken);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Traps/Shuriken/texture.png",
        "../assets/Sprites/Enemies/Traps/Shuriken/texture.json",
        nullptr);

    mRigidBodyComponent = new RigidBodyComponent(
        this, 1.0f, 0.0f, false);

    mDrawComponent->AddAnimation("spinning", 0, 7);

    mDrawComponent->SetAnimation("spinning");

    SetPosition(position);
    mBehaviorState = BehaviorState::Moving;
}

void Shuriken::ManageState()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Moving:
        break;
    default:
        mBehaviorState = BehaviorState::Moving;
        break;
    }
}

void Shuriken::ManageAnimations()
{
    switch (mBehaviorState)
    {
    default:
        mDrawComponent->SetAnimation("spinning");
        mDrawComponent->SetAnimFPS(30.f);
        break;
    }
}

void Shuriken::OnUpdate(float deltaTime)
{
    ManageState();
    ManageAnimations();
}
