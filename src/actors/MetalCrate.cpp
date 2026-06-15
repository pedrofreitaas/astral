#include "./MetalCrate.h"
#include "./Zoe.h"

MetalCrate::MetalCrate(
    Game *game,
    const Vector2& center
) : Actor(game, 1, false, "MetalCrate")
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 8.f, true);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/MetalCrate/texture.png",
        "../assets/Sprites/MetalCrate/texture.json",
        std::bind(&MetalCrate::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player)-1);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("freezing", 1, 4, false);
    mDrawComponent->SetAnimation("idle");
    
    mColliderComponent = new AABBColliderComponent(
        this,
        5, 5,
        21, 27,
        ColliderLayer::MetalCrate);

    mColliderComponent->SetIgnoreLayers({ColliderLayer::Nevasca}, IgnoreOption::IgnoreResolution);

    SetPosition(center - GetHalfSize());
    SetBehaviorState(BehaviorState::Idle);

    mSpawnCenter = center;
}

void MetalCrate::AnimationEndCallback(std::string animationName)
{
}

void MetalCrate::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (IsFrozen()) return;

    if (other->GetLayer() == ColliderLayer::Nevasca)
    {
        IncreaseFreezing();
        return;
    }
    
    if (other->GetLayer() == ColliderLayer::Player) {
        float force = mGame->GetConfig()->Get<float>("METAL_CRATE_PUSH_FORCE");
        mRigidBodyComponent->ApplyForce(Vector2(-minOverlap * force, 0.f));
        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        Zoe *zoe = mGame->GetZoe();

        if (zoe->IsChargedPlayerAttack()) {
            TakeKnockback(Vector2(0.f, -500.f));
        }

        return;
    }
}

void MetalCrate::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Nevasca && !IsFrozen())
    {
        IncreaseFreezing();
        return;
    }
}

void MetalCrate::ManageAnimations()
{
    switch (GetBehaviorState())
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(8.f);
        break;

    case BehaviorState::Frozen:
        mDrawComponent->SetAnimation("freezing");
        mDrawComponent->SetAnimFPS(12.f);
        break;

    default:
        break;
    }
}

bool MetalCrate::IsInInvalidPosition()
{
    auto player = GetGame()->GetZoe();
    return mColliderComponent->IsCloseToTileWallHorizontally(player->GetWidth());
}

void MetalCrate::OnUpdate(float deltaTime)
{
    Actor::OnUpdate(deltaTime);
    ManageAnimations();

    if ( IsInInvalidPosition() )
    {
        SetPosition(mSpawnCenter - GetHalfSize());
        mRigidBodyComponent->ResetVelocity();
    }
}

void MetalCrate::Freeze()
{
    if (IsFrozen()) return;

    SetBehaviorState(BehaviorState::Frozen);
    mRigidBodyComponent->SetEnabled(false);
}

void MetalCrate::StopFreeze()
{
    if (!IsFrozen()) return;

    SetBehaviorState(BehaviorState::Idle);
    mRigidBodyComponent->SetEnabled(true);
}