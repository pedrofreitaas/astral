#include "./Father.h"

Father::Father(
    Game *game,
    const Vector2& center
) : Actor(game, 1, false, "Father")
{
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Father/texture.png",
        "../assets/Sprites/Father/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player)-1);

    mDrawComponent->AddAnimation("idle", 0, 5);
    mDrawComponent->AddAnimation("moving", 6, 12);
    mDrawComponent->SetAnimation("idle");
    
    mColliderComponent = new AABBColliderComponent(
        this,
        6, 6,
        15, 24,
        ColliderLayer::Father);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 10.0f);

    SetPosition(center - GetHalfSize());
    SetBehaviorState(BehaviorState::Idle);

    mGame->SetFather(this);
}

void Father::ManageAnimations()
{
    switch (GetBehaviorState())
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(6.f);
        break;

    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(10.f);
        break;

    default:
        break;
    }
}

void Father::OnUpdate(float deltaTime)
{
    Actor::OnUpdate(deltaTime);

    if (mRigidBodyComponent->GetVelocity().x > 0.0f)
        SetRotation(0.0f);
    else if (mRigidBodyComponent->GetVelocity().x < 0.0f)
        SetRotation(Math::Pi);

    bool moving = mRigidBodyComponent->GetVelocity().x != 0.0f || mRigidBodyComponent->GetVelocity().y != 0.0f;

    if (moving && GetBehaviorState() != BehaviorState::Moving)
        SetBehaviorState(BehaviorState::Moving);
    else if (!moving && GetBehaviorState() != BehaviorState::Idle)
        SetBehaviorState(BehaviorState::Idle);

    ManageAnimations();
}