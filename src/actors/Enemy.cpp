#include "Enemy.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/ai/AIMovementComponent.h"
#include "Zoe.h"
#include "Actor.h"
#include "Collider.h"

Enemy::Enemy(Game *game, const Vector2 &position)
    : Actor(game)
{
    mGame->AddEnemy(this);
}

Enemy::~Enemy()
{
    mGame->RemoveEnemy(this);
}

void Enemy::OnUpdate(float deltaTime)
{
    ManageState();
    ManageAnimations();
    mColliderComponent->MaintainInMap();

    Actor::OnUpdate(deltaTime);
}

bool Enemy::PlayerOnSight(float distance, float angle)
{
    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 lineOfSightStart = GetCenter();

    float dir = GetRotation() == 0.f ? 1.f : -1.f;

    Vector2 viewDir = Vector2(dir, 0.f);
    viewDir = viewDir.Rotate(viewDir, angle);
    viewDir.Normalize();

    Vector2 lineOfSightEnd = lineOfSightStart + viewDir*distance;

    auto zoeCollider = zoe->GetComponent<AABBColliderComponent>();

    if (zoeCollider == nullptr)
        return false;

    bool isIntersecting = zoeCollider->IsSegmentIntersecting(lineOfSightStart, lineOfSightEnd);

    return isIntersecting;
}

bool Enemy::PlayerOnFov(float minDistance, float maxDistance)
{
    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 toZoe = zoe->GetPosition() - GetPosition();
    
    if (toZoe.LengthSq() > maxDistance * maxDistance) return false;
    if (toZoe.LengthSq() < minDistance * minDistance) return true;

    toZoe.Normalize();

    Vector2 forward = GetForward();

    float dot = Vector2::Dot(forward, toZoe);
    float angle = Math::Acos(dot);

    const float fovAngle = Math::Pi / 4.f; // 45 degrees field of view

    return angle < fovAngle;
}

std::vector<SDL_Rect> Enemy::GetPath() const
{
    if (mAIMovementComponent == nullptr)
        return {};

    return mAIMovementComponent->GetPath();
}

Vector2 Enemy::GetCurrentAppliedForce(float modifier)
{
    const RigidBodyComponent* rb = GetComponent<RigidBodyComponent>();

    if (!rb)
        return Vector2::Zero;

    return rb->GetAppliedForce() * modifier;
}

void Enemy::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnHorizontalCollision(minOverlap, other);

    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        Fireball *fireball = static_cast<Fireball*>(other->GetOwner());
        Vector2 fireballCenter = fireball->GetCenter();

        Vector2 knockbackDir = GetCenter() - fireballCenter;
        knockbackDir.Normalize();

        TakeDamage(knockbackDir * Enemy::FIREBALL_KNOCKBACK_FORCE);
        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        TakeDamage(Vector2(-minOverlap, 1.f) * Enemy::PLAYER_ATTACK_KNOCKBACK_FORCE);
        return;
    }
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnVerticalCollision(minOverlap, other);

    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        Fireball *fireball = static_cast<Fireball*>(other->GetOwner());
        Vector2 fireballCenter = fireball->GetCenter();

        Vector2 knockbackDir = GetCenter() - fireballCenter;
        knockbackDir.Normalize();

        TakeDamage(knockbackDir * Enemy::FIREBALL_KNOCKBACK_FORCE);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Player && minOverlap < 0.f)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        TakeDamage();
        return;
    }
}
