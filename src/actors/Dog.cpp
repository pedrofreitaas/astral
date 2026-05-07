#include "Dog.h"

Dog::Dog(Game *game, const Vector2 &position)
    : Item(
          game,
          position,
          "../assets/Sprites/Items/Dog/texture.png",
          "../assets/Sprites/Items/Dog/texture.json",
          30, 20,
          nullptr,
          Button::A,
          0, 9, 6, false, true) 
{
    mDrawComponent->AddAnimation("licking", 10, 13);
    mDrawComponent->AddAnimation("barking", 14, 16);

    SetBehaviorState(BehaviorState::Idle);
    mDrawComponent->SetAnimationEndCallback([this](const std::string &animationName) {
        AnimationEndCallback(animationName);
    });
}

void Dog::OnPick()
{
    PickCallback();
}

void Dog::OnUpdate(float deltaTime)
{
    Item::OnUpdate(deltaTime);

    float myX = GetPosition().x;
    float zoeX = mGame->GetZoe()->GetPosition().x;

    SetRotation(zoeX > myX ? 0.f : Math::Pi);

    mIsPickable = mIsPickable & (mBehaviorState == BehaviorState::Idle);

    ManageAnimations();
}

void Dog::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;

    case BehaviorState::Barking:
        mDrawComponent->SetAnimation("barking");
        break;

    case BehaviorState::Licking:
        mDrawComponent->SetAnimation("licking");
        break;

    default:
        break;
    }
}

void Dog::PickCallback()
{
    if (mBehaviorState != BehaviorState::Idle)
        return;
    
    float random = Math::RandRange(0.f, 1.f);

    if (random < 0.5f)
        Bark();
    else
        Lick();
}

void Dog::AnimationEndCallback(const std::string &animationName)
{
    if (animationName == "barking")
        SetBehaviorState(BehaviorState::Idle);
    
    else if (animationName == "licking")
        SetBehaviorState(BehaviorState::Idle);
}

void Dog::Bark()
{
    mDrawComponent->SetAnimation("barking");
    SetBehaviorState(BehaviorState::Barking);

    mGame->GetAudio()->PlaySound("dogBark.mp3");
}

void Dog::Lick()
{
    mDrawComponent->SetAnimation("licking");
    SetBehaviorState(BehaviorState::Licking);
}