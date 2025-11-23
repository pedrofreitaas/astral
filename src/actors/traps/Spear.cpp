#include "Spear.h"

Spear::Spear(Game *game, const Vector2 &position)
    : Actor(game, 1.f), mTipCollider(nullptr)
{
    mTimerComponent = new TimerComponent(this);

    mColliderComponent = new AABBColliderComponent(
        this, 4, 60, 8, 3, ColliderLayer::SpearBlock);

    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::PlayerAttack
    });

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Traps/Spear/texture.png",
        "../assets/Sprites/Enemies/Traps/Spear/texture.json",
        [this](std::string animationName)
        { AnimationEndCallback(animationName); });

    mRigidBodyComponent = new RigidBodyComponent(
        this, 1.0f, 0.0f, true);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("spiking", 1, 7);

    mDrawComponent->SetAnimation("idle");

    mTimerComponent->AddTimer(Spear::TRIGGER_COOLDOWN, [this]()
                              { Trigger(); });

    mTipCollider = new Collider(
        mGame,
        this,
        GetPosition() + Vector2(5, 49),
        Vector2(7, 10),
        [this](bool collided, const float minOverlap, AABBColliderComponent *other)
        {},
        DismissOn::None,
        ColliderLayer::SpearTip,
        {ColliderLayer::SpearBlock, ColliderLayer::PlayerAttack},
        -1.f);

    Vector2 toTipCenter = mTipCollider->GetCenter() - GetPosition();

    SetPosition(position-toTipCenter);
}

void Spear::ManageState()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        break;
    case BehaviorState::Attacking:
        break;
    default:
        mBehaviorState = BehaviorState::Idle;
        break;
    }
}

void Spear::AnimationEndCallback(std::string animationName)
{
    if (animationName == "spiking")
    {
        mBehaviorState = BehaviorState::Idle;
        mTimerComponent->AddTimer(Spear::TRIGGER_COOLDOWN, [this]()
                                  { Trigger(); });
    }
}

void Spear::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(1.f);

        mTipCollider->SetPosition(GetPosition() + Vector2(5, 49));

        break;
    case BehaviorState::Attacking:
        mDrawComponent->SetAnimation("spiking");
        mDrawComponent->SetAnimFPS(7.f);

        switch (mDrawComponent->GetCurrentSprite()) // seven frame animation
        {
        case 0:
            mTipCollider->SetPosition(GetPosition() + Vector2(5, 49));
            break;
        case 1:
            mTipCollider->SetPosition(GetPosition() + Vector2(5, 2));
            break;
        case 5:
            mTipCollider->SetPosition(GetPosition() + Vector2(5, 4));
            break;
        case 6:
            mTipCollider->SetPosition(GetPosition() + Vector2(5, 22));
            break;
        default:
            break;
        }

        break;
    default:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(1.f);
        break;
    }
}

void Spear::OnUpdate(float deltaTime)
{
    ManageState();
    ManageAnimations();
}

void Spear::Trigger()
{
    if (mBehaviorState == BehaviorState::Attacking)
    {
        return;
    }

    mBehaviorState = BehaviorState::Attacking;
}

Vector2 Spear::GetTipCenter() const
{
    return mTipCollider->GetCenter();
}