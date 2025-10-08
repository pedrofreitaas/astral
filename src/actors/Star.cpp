#include "./Star.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "./Zoe.h"

Star::Star(Game *game) : 
    Actor(game)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f);
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->SetApplyFriction(false);
    
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Star/texture.png",
        "../assets/Sprites/Star/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("twinkle", {0});
    mDrawComponent->SetAnimation("twinkle");
    mDrawComponent->SetAnimFPS(1.0f);
    mDrawComponent->Scale(3);
}

void Star::ManageState()
{
    const Zoe* zoe = mGame->GetZoe();

    if (!zoe)
        return;

    Vector2 toZoe = zoe->GetPosition() - GetPosition();
    float distanceToZoe = toZoe.Length();

    switch (mBehaviorState)
    {
        case BehaviorState::Fleeing:
            break;
        case BehaviorState::Provoking:
            break;
        default:
            mBehaviorState = BehaviorState::Idle;    
            break;
    }
}

void Star::OnUpdate(float deltaTime)
{
    ManageState();
}