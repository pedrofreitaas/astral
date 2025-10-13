#include "Enemy.h"
#include "../core/Game.h"
#include "Zoe.h"
#include "Actor.h"
#include "../components/draw/DrawAnimatedComponent.h"

Enemy::Enemy(Game *game, float forwardSpeed, const Vector2 &position, float fowardSpeed)
    : Actor(game), mFowardSpeed(fowardSpeed)
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
        std::bind(&Enemy::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Enemy) + 1);

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

    float distanceToZoe = (zoe->GetPosition() - GetPosition()).Length();

    if (distanceToZoe > 450.f)
    {
        mBehaviorState = BehaviorState::Asleep;
        return;
    }

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        if (distanceToZoe < 200.0f)
            mBehaviorState = BehaviorState::Waking;
        break;
    case BehaviorState::Waking:
        break;
    case BehaviorState::Idle:
        break;
    case BehaviorState::Moving:
    {
        mRigidBodyComponent->ApplyForce(Vector2(mFowardSpeed, 0.0f));
        if (mRigidBodyComponent->GetVelocity().x > 0.f)
            SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < 0.f)
            SetRotation(Math::Pi);
        break;
    }
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
        break;
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
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
        mFowardSpeed *= -1;
    }
}

void Enemy::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
}