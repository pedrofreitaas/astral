#include "Spear.h"

Spear::Spear(Game *game, const Vector2 &position, bool inversed)
    : Actor(game, 1.f), mTipCollider(nullptr), mIsInversed(inversed)
{
    mTimerComponent = new TimerComponent(this);

    if (mIsInversed) {
        mColliderComponent = new AABBColliderComponent(
            this, 4, 0, 8, 3, ColliderLayer::SpearBlock);
    }

    else {
        mColliderComponent = new AABBColliderComponent(
            this, 4, 60, 8, 3, ColliderLayer::SpearBlock);
    }

    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::PlayerAttack,
        ColliderLayer::Nevasca
    });

    mDrawComponent = new DrawAnimatedComponent(
        this,
        mIsInversed 
         ? "../assets/Sprites/Enemies/Traps/SpearInversed/texture.png" 
         : "../assets/Sprites/Enemies/Traps/Spear/texture.png",
        mIsInversed 
         ? "../assets/Sprites/Enemies/Traps/SpearInversed/texture.json"
         : "../assets/Sprites/Enemies/Traps/Spear/texture.json",
        [this](std::string animationName)
        { AnimationEndCallback(animationName); });

    mRigidBodyComponent = new RigidBodyComponent(
        this, 1.0f, 0.0f, !mIsInversed);

    mDrawComponent->AddAnimation("idle", {0});
    mDrawComponent->AddAnimation("spiking", 1, 7);

    mDrawComponent->SetAnimation("idle");

    float cooldown = mGame->GetConfig()->Get<float>("SPEAR_COOLDOWN");
    
    mTimerComponent->AddTimer(cooldown, [this]()
                              { Trigger(); });

    mTipCollider = new Collider(
        mGame,
        this,
        GetPosition() + (mIsInversed ? Vector2(4, 3) : Vector2(5, 49)),
        Vector2(7, 10),
        [this](bool collided, const float minOverlap, AABBColliderComponent *other)
        {},
        DismissOn::None,
        ColliderLayer::SpearTip,
        {ColliderLayer::SpearBlock, ColliderLayer::PlayerAttack, ColliderLayer::Nevasca},
        -1.f);

    SetPosition(position-GetHalfSize());
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
        SetBehaviorState(BehaviorState::Idle);
        break;
    }
}

void Spear::AnimationEndCallback(std::string animationName)
{
    if (animationName == "spiking")
    {
        SetBehaviorState(BehaviorState::Idle);
        float cooldown = mGame->GetConfig()->Get<float>("SPEAR_COOLDOWN");
        mTimerComponent->AddTimer(cooldown, [this]()
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

        mTipCollider->SetPosition(GetPosition() + (mIsInversed ? Vector2(4, 3) : Vector2(5, 49)));

        break;
    case BehaviorState::Attacking:
        mDrawComponent->SetAnimation("spiking");
        mDrawComponent->SetAnimFPS(7.f);

        switch (mDrawComponent->GetCurrentSprite()) // seven frame animation
        {
        case 0:
            if (!mIsInversed) mTipCollider->SetPosition(GetPosition() + Vector2(5, 49));
            else mTipCollider->SetPosition(GetPosition() + Vector2(4, 4));
            break;
        case 1:
            if (!mIsInversed) mTipCollider->SetPosition(GetPosition() + Vector2(5, 2));
            else mTipCollider->SetPosition(GetPosition() + Vector2(4, 52));
            break;
        case 5:
            if (!mIsInversed) mTipCollider->SetPosition(GetPosition() + Vector2(5, 4));
            else mTipCollider->SetPosition(GetPosition() + Vector2(4, 50));
            break;
        case 6:
            if (!mIsInversed) mTipCollider->SetPosition(GetPosition() + Vector2(5, 22));
            else mTipCollider->SetPosition(GetPosition() + Vector2(4, 32));
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

    SetBehaviorState(BehaviorState::Attacking);
}

Vector2 Spear::GetTipCenter() const
{
    return mTipCollider->GetCenter();
}