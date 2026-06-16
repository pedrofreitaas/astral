#include "Enemy.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/ai/AIMovementComponent.h"
#include "Zoe.h"
#include "Actor.h"
#include "Collider.h"

Enemy::Enemy(Game *game, const Vector2 &position, float maxSeeDistance, float minSeeDistance)
    : Actor(game), mMaxSeeDistance(maxSeeDistance), 
    mMinSeeDistance(minSeeDistance), mHasSeenPlayerThisFrame(false),
    mLastSeenPlayerCenter(Vector2::Zero), mSpawnPosition(position),
    mHowLongLastSeenPlayer(99999.f), mPlayerOnSightThisFrame(false),
    mLastSeenPlayerDistanceSquared(0.f), mDistanceToPlayerSquared(0.f)
{
    mGame->AddEnemy(this);
}

Enemy::~Enemy()
{
    mGame->RemoveEnemy(this);
}

void Enemy::OnUpdate(float deltaTime)
{
    mHasSeenPlayerThisFrame = PlayerOnFov();
    mPlayerOnSightThisFrame = PlayerOnSight();
    mDistanceToPlayerSquared = (GetGame()->GetZoe()->GetCenter() - GetCenter()).LengthSq();
    
    if (mHasSeenPlayerThisFrame) {
        mLastSeenPlayerCenter = GetGame()->GetZoe()->GetCenter();
        mLastSeenPlayerDistanceSquared = mDistanceToPlayerSquared;
        mHowLongLastSeenPlayer = 0.f;
    }
    else {
        mHowLongLastSeenPlayer += deltaTime;
    }
    
    ManageState();
    ManageAnimations();
    mColliderComponent->MaintainInMap();

    Actor::OnUpdate(deltaTime);
}

bool Enemy::PlayerOnSight(float angle)
{
    float distance = mMaxSeeDistance;
    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 lineOfSightStart = GetCenter();

    float dir = GetRotation() == 0.f ? 1.f : -1.f;

    Vector2 viewDir = Vector2(dir, 0.f);
    viewDir = viewDir.Rotate(viewDir, angle);
    viewDir.Normalize();

    Vector2 lineOfSightEnd = lineOfSightStart + viewDir*distance;

    return mColliderComponent->IsSegmentIntersectingPlayerLayer(lineOfSightStart, lineOfSightEnd);
}

bool Enemy::PlayerOnFov()
{
    float minDistance = mMinSeeDistance;
    float maxDistance = mMaxSeeDistance;

    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 toZoe = zoe->GetCenter() - GetCenter();
    float distanceToZoeSq = GetDistanceToPlayerSquared();

    if (distanceToZoeSq > maxDistance * maxDistance) return false;

    toZoe.Normalize();

    Vector2 forward = GetForward();

    float dot = Vector2::Dot(forward, toZoe);
    float angle = Math::Acos(dot);

    const float fovAngle = mGame->GetConfig()->Get<float>("ENEMY.FOV_ANGLE");

    bool isInFov = angle < fovAngle;
    bool withinMinimalDistance = distanceToZoeSq < minDistance * minDistance;
    bool canSeePlayerNoColliderInTheMiddle = mColliderComponent->
        IsSegmentIntersectingPlayerLayer(GetCenter(), zoe->GetCenter());
    
    return (isInFov || withinMinimalDistance) && canSeePlayerNoColliderInTheMiddle;
}

Vector2 Enemy::GetCurrentAppliedForce(float modifier) const
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
        TakeDamage();
        TakeKnockback(
            Vector2(Math::Sign(-minOverlap) * mGame->GetConfig()->Get<float>("ZOE.POWERS.FIREBALL.KNOCKBACK_FORCE"), 0.f)
        );
        
        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {
        TakeDamage();

        TakeKnockback(
            Vector2(
                Math::Sign(-minOverlap) * mGame->GetConfig()->Get<float>("ENEMY.PLAYER_KNOCKBACK_FORCE"), 
                0.f
            )
        );
        return;
    }

    if (other->GetLayer() == ColliderLayer::Player)
    {
        // just to avoid player stuck on enemy when colliding horizontally.
        TakeKnockback(
            Vector2(Math::Sign(-minOverlap) * mGame->GetConfig()->Get<float>("ENEMY.PLAYER_KNOCKBACK_FORCE"), 0.f)
        );
        return;
    }
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    mAIMovementComponent->OnVerticalCollision(minOverlap, other);

    if (other->GetLayer() == ColliderLayer::Fireball)
    {
        TakeDamage();
        // dont apply knockback on stomp.
        return;
    }

    // stomp behavior
    if (
        other->GetLayer() == ColliderLayer::Player && 
        minOverlap > 0.f &&
        mAIMovementComponent != nullptr &&
        mAIMovementComponent->GetMovementType() == TypeOfMovement::Walker
    )
    {
        Vector2 dist = GetCenter() - other->GetCenter();
        dist.Normalize();

        // just to avoid enemy stuck above damaging player on stomp.
        TakeKnockback(dist * mGame->GetConfig()->Get<float>("ENEMY.PLAYER_KNOCKBACK_FORCE"));

        return;
    }

    if (other->GetLayer() == ColliderLayer::PlayerAttack)
    {               
        TakeDamage();

        // min overlap here is vertical, not left or right
        int left = GetCenter().x < other->GetCenter().x ? -1 : 1;

        TakeKnockback(
            Vector2(
                left * mGame->GetConfig()->Get<float>("ENEMY.PLAYER_KNOCKBACK_FORCE"), 
                0.f
            )
        );
        return;
    }
}

void Enemy::Freeze()
{
    if (IsFrozen()) return;

    SetBehaviorState(BehaviorState::Frozen);
    mAIMovementComponent->SetEnabled(false);
}

void Enemy::StopFreeze()
{
    if (!IsFrozen()) return;

    SetBehaviorState(BehaviorState::Moving);
    mAIMovementComponent->SetEnabled(true);
}

SDL_Rect Enemy::GetThreatRect() const
{
    return SDL_Rect{
        static_cast<int>(GetCenter().x - GetWidth()*1.4),
        static_cast<int>(GetCenter().y - GetHeight()*1.4),
        static_cast<int>(GetWidth()*2.8),
        static_cast<int>(GetHeight()*2.8)
    };
}

std::vector<Vector2> Enemy::GetObstaclesAroundCenters() const
{
    return mAIMovementComponent->GetObstaclesAroundCenters();
}

Vector2 Enemy::GetCurrentVelocity(float modifier) const
{
    const RigidBodyComponent* rb = GetComponent<RigidBodyComponent>();

    if (!rb)
        return Vector2::Zero;

    return rb->GetVelocity() * modifier;
}

float Enemy::GetFovAngle() const
{
    return mGame->GetConfig()->Get<float>("ENEMY.FOV_ANGLE");
}

bool Enemy::isAISeeking() const
{
    return mAIMovementComponent &&
           mAIMovementComponent->GetMovementState() == MovementState::Seeking;
}