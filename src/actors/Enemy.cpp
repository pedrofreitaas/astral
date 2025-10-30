#include "Enemy.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/ai/AIMovementComponent.h"
#include "Zoe.h"
#include "Actor.h"

Enemy::Enemy(Game *game, float forwardSpeed, const Vector2 &position, float fowardSpeed)
    : Actor(game)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f);
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 8,
        10, 20,
        ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Zod/texture.png",
        "../assets/Sprites/Enemies/Zod/texture.json",
        std::bind(&Enemy::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);
    
    mAIMovementComponent = new AIMovementComponent(this, fowardSpeed, .01f);

    mDrawComponent->AddAnimation("asleep", {0});
    mDrawComponent->AddAnimation("waking", 1, 2);
    mDrawComponent->AddAnimation("idle", 3, 5);
    mDrawComponent->AddAnimation("damage", 6, 7);
    mDrawComponent->AddAnimation("moving", 8, 15);
    mDrawComponent->AddAnimation("charging", 16, 19);
    mDrawComponent->AddAnimation("dying", 20, 25);

    mDrawComponent->SetAnimation("asleep");

    mBehaviorState = BehaviorState::Asleep;
    SetPosition(position);
}

void Enemy::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool isInFov = PlayerOnFov();
    float distanceToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        if (distanceToZoe < 200.0f*200.f)
            mBehaviorState = BehaviorState::Waking;
        break;
    case BehaviorState::Waking:
        break;
    case BehaviorState::Idle:
        break;
    case BehaviorState::Moving:
    {
        if (mRigidBodyComponent->GetVelocity().x > 0.f)
            SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < 0.f)
            SetRotation(Math::Pi);

        // if (!isInFov) mAIMovementComponent->SeekPlayer();
        if (PlayerOnSight()) mBehaviorState = BehaviorState::Charging;
        
        break;
    }
    case BehaviorState::Charging:
        if (!PlayerOnSight()) mBehaviorState = BehaviorState::Moving;
        break;

    default:
        mBehaviorState = BehaviorState::Asleep;
        break;
    }
}

void Enemy::AnimationEndCallback(std::string animationName)
{
    if (animationName == "waking")
    {
        mBehaviorState = BehaviorState::Moving;
        return;
    }

    if (animationName == "charging")
    {
        mBehaviorState = BehaviorState::Moving;
    }
}

void Enemy::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        mDrawComponent->SetAnimation("asleep");
        break;
    case BehaviorState::Waking:
        mDrawComponent->SetAnimation("waking");
        mDrawComponent->SetAnimFPS(3.f);
        break;
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(6.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    default:
        mDrawComponent->SetAnimation("asleep");
        break;
    }
}

void Enemy::OnUpdate(float deltaTime)
{
    ManageState();
    ManageAnimations();
    mColliderComponent->MaintainInMap();
}

void Enemy::TakeDamage()
{
}

void Enemy::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mAIMovementComponent->SetFowardSpeed(-mAIMovementComponent->GetFowardSpeed());
    }
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
}

bool Enemy::PlayerOnSight()
{
    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 lineOfSightStart = GetCenter();

    float dir = GetRotation() == 0.f ? 1.f : -1.f;
    Vector2 lineOfSightEnd = lineOfSightStart + Vector2(100.f * dir, 0.f);

    auto zoeCollider = zoe->GetComponent<AABBColliderComponent>();

    if (zoeCollider == nullptr)
        return false;

    bool isIntersecting = zoeCollider->IsSegmentIntersecting(lineOfSightStart, lineOfSightEnd);

    return isIntersecting;
}

bool Enemy::PlayerOnFov()
{
    auto zoe = GetGame()->GetZoe();

    if (zoe == nullptr)
        return false;

    Vector2 toZoe = zoe->GetPosition() - GetPosition();
    
    const float maxDist = 400.f;
    const float minDist = 20.f;
    if (toZoe.LengthSq() > maxDist * maxDist) return false;
    if (toZoe.LengthSq() < minDist * minDist) return true;

    toZoe.Normalize();

    Vector2 forward = GetForward();

    float dot = Vector2::Dot(forward, toZoe);
    float angle = Math::Acos(dot);

    const float fovAngle = Math::Pi / 4.f; // 45 degrees field of view

    return angle < fovAngle;
}