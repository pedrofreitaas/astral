#include "Enemy.h"

Enemy::Enemy(Game* game, float forwardSpeed, const Vector2& position)
    : Actor(game)
{
}

void Enemy::OnUpdate(float deltaTime)
{
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