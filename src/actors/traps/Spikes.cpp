#include "Spikes.h"

Spikes::Spikes(Game *game, const Vector2 &position)
    : Actor(game, 1.f)
{
    mTimerComponent = new TimerComponent(this);

    mColliderComponent = new AABBColliderComponent(
        this, 0, 28, 32, 4, ColliderLayer::SpikesBlock);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Traps/Spikes/texture.png",
        "../assets/Sprites/Enemies/Traps/Spikes/texture.json",
        [this](std::string animationName)
        { AnimationEndCallback(animationName); });

    mRigidBodyComponent = new RigidBodyComponent(
        this, 1.0f, 0.0f);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("spiking", 1, 9);

    mDrawComponent->SetAnimation("idle");

    mTimerComponent->AddTimer(Spikes::TRIGGER_COOLDOWN, [this]()
                              { Trigger(); });

    SetPosition(position);

    mSpikeCollider = new Collider(
        mGame,
        this,
        GetPosition() + Vector2(0, 19),
        Vector2(32, 13),
        [this](bool collided, const float minOverlap, AABBColliderComponent *other) {},
        DismissOn::None,
        ColliderLayer::Spikes,
        {ColliderLayer::SpikesBlock},
        -1.f);
    
    mSpikeCollider->SetEnabled(false);
}

void Spikes::ManageState()
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

void Spikes::AnimationEndCallback(std::string animationName)
{
    if (animationName == "spiking")
    {
        mBehaviorState = BehaviorState::Idle;
        mSpikeCollider->SetEnabled(false);

        mTimerComponent->AddTimer(
            Spikes::TRIGGER_COOLDOWN, [this]()
            { Trigger(); });
    }
}

void Spikes::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(1.f);
        break;
    case BehaviorState::Attacking:
        mDrawComponent->SetAnimation("spiking");
        mDrawComponent->SetAnimFPS(9.f);
        break;
    default:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(1.f);
        break;
    }
}

void Spikes::OnUpdate(float deltaTime)
{
    mSpikeCollider->SetPosition(GetPosition() + Vector2(0, 19));

    ManageState();
    ManageAnimations();
}

void Spikes::Trigger()
{
    if (mBehaviorState == BehaviorState::Attacking)
    {
        return;
    }

    mBehaviorState = BehaviorState::Attacking;
    mSpikeCollider->SetEnabled(true);
}

Vector2 Spikes::GetBaseCenter() const
{
    return GetPosition() + Vector2(16, 32);
}