#include "./Crate.h"

Crate::Crate(
    Game *game,
    const Vector2& center
) : Actor(game, 1, false, "crate")
{
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Crate/texture.png",
        "../assets/Sprites/Crate/texture.json",
        std::bind(&Crate::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player)-1);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("breaking", 0, 8);
    mDrawComponent->SetAnimation("idle");
    
    new AABBColliderComponent(
        this,
        11, 11,
        13, 18,
        ColliderLayer::Crate);

    SetPosition(center - GetHalfSize());
    SetBehaviorState(BehaviorState::Idle);
}

void Crate::AnimationEndCallback(std::string animationName)
{
    if (animationName == "breaking")
    {
        SetState(ActorState::Destroy);
    }
}

void Crate::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        SetBehaviorState(BehaviorState::Dying);
    }
}

void Crate::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        SetBehaviorState(BehaviorState::Dying);
    }
}

void Crate::ManageAnimations()
{
    switch (GetBehaviorState())
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(8.f);
        break;

    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("breaking");
        mDrawComponent->SetAnimFPS(12.f);
        break;

    default:
        break;
    }
}

void Crate::OnUpdate(float deltaTime)
{
    Actor::OnUpdate(deltaTime);
    ManageAnimations();
}