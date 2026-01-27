#include "Nevasca.h"

Nevasca::Nevasca(
    Game* game, Vector2 position, 
    Vector2 direction, Actor* shooter
) : Projectile(game, position, direction, 0.f, shooter, 1.5f)
{
    mType = "Nevasca";
    
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f, 1.0f, true);
    mRigidBodyComponent->SetGravityScale(.3f);

    mColliderComponent = new AABBColliderComponent(
        this, 
        4, 4, 8, 8,
        ColliderLayer::Nevasca);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this, 
        "../assets/Sprites/Zoe/Nevasca/texture.png", 
        "../assets/Sprites/Zoe/Nevasca/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player) - 1);

    mDrawAnimatedComponent->AddAnimation("0", {0});
    mDrawAnimatedComponent->AddAnimation("1", {1});
    mDrawAnimatedComponent->AddAnimation("2", {2});
    mDrawAnimatedComponent->AddAnimation("3", {3});
    mDrawAnimatedComponent->AddAnimation("4", {4});

    int chosenSnowFlake = Math::RandRangeInt(0, 4);
    std::string snowFlakeStr = std::to_string(chosenSnowFlake);

    mDrawAnimatedComponent->SetAnimation(snowFlakeStr);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);
    mDrawAnimatedComponent->SetAnimFPS(0.f);

    SetPosition(position - mDrawAnimatedComponent->GetHalfSpriteSize());
    mDirection = direction;
    mDirection.Normalize();

    mRigidBodyComponent->ApplyImpulse(mDirection *
        game->GetConfig()->Get<float>("ZOE.POWERS.NEVASCA.SPEED"));
}

void Nevasca::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
}   

void Nevasca::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void Nevasca::Kill()
{
    mGame->RemoveActor(this);
}

void Nevasca::OnUpdate(float deltaTime)
{
    Projectile::OnUpdate(deltaTime);
}