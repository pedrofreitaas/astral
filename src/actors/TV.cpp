#include "TV.h"

TV::TV(Game *game, const Vector2 &position)
    : Item(
          game,
          position,
          "../assets/Sprites/Items/TV/texture.png",
          "../assets/Sprites/Items/TV/texture.json",
          48, 16,
          std::bind(&TV::OnPick, this),
          Button::A,
          0, 0, 1, false, true), 
    mTurnOnTimer(nullptr)
{
    mDrawComponent->SetDrawOrder(static_cast<int>(DrawLayerPosition::DetailsDown));

    mDrawComponent->AddAnimation("on", {1});
    SetBehaviorState(BehaviorState::Idle);

    mTimerComponent = new TimerComponent(this);
}

void TV::OnPick()
{
    if (
        mTurnOnTimer != nullptr &&
        mTimerComponent->checkTimerRemaining(mTurnOnTimer) > 0.f
    )
    {
        return;
    }

    mGame->GetAudio()->PlaySound("PickItem.wav");

    SetBehaviorState(mBehaviorState == BehaviorState::Idle ? BehaviorState::Moving : BehaviorState::Idle);

    if (mTurnOnTimer == nullptr)
        mTurnOnTimer = mTimerComponent->AddNotRemovableTimer(.5f, nullptr);
    else
        mTimerComponent->Restart(mTurnOnTimer);
}

void TV::OnUpdate(float deltaTime)
{
    Item::OnUpdate(deltaTime);
    ManageAnimations();
}

void TV::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;

    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("on");
        break;
    default:
        break;
    }
}
