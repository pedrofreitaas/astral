#include "./Torch.h"

Torch::Torch(
    Game *game,
    const Vector2& center
) : Actor(game, 1, false, "torch")
{
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Torch/texture.png",
        "../assets/Sprites/Torch/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player) -1);

    mDrawComponent->AddAnimation("idle", 0, 28);
    mDrawComponent->SetAnimation("idle");
    
    new AABBColliderComponent(
        this,
        0, 0,
        32, 32,
        ColliderLayer::Torch,
        false);

    SetPosition(center - GetHalfSize());
    SetBehaviorState(BehaviorState::Idle);
}

void Torch::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        mGame->SetCheckpoint(GetCenter());
        return;
    }
}

void Torch::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        mGame->SetCheckpoint(GetCenter());
        return;
    }
}

void Torch::ManageAnimations()
{
    switch (GetBehaviorState())
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(8.f);
        break;

    default:
        break;
    }
}

void Torch::OnUpdate(float deltaTime)
{
    Actor::OnUpdate(deltaTime);
    ManageAnimations();
}