#include "Enemy.h"
#include "../components/draw/DrawAnimatedComponent.h"

Enemy::Enemy(Game* game, float forwardSpeed, const Vector2& position)
    : Actor(game)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this, 
        19, 5, 
        27, 20, 
        ColliderLayer::Enemy, 
        true);

    mDrawComponent = new DrawAnimatedComponent(
        this, 
        "../assets/Sprites/Enemies/Zod/texture.png",
        "../assets/Sprites/Enemies/Zod/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Enemy) + 1);
    
    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->SetAnimation("asleep");

    SetPosition(position);
}

void Enemy::OnUpdate(float deltaTime)
{
    ManageState();
    MaintainInbound();
    ManageAnimations();
}

void Enemy::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void Enemy::ManageState()
{
}

void Enemy::MaintainInbound()
{
}

void Enemy::ManageAnimations()
{
}

void Enemy::TakeDamage()
{
}