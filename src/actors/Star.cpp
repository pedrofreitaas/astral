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

    mColliderComponent = new AABBColliderComponent(
        this, 0, 0, 33, 33,
        ColliderLayer::Player
    );

    mGame->SetStar(this);
}

void Star::ManageState()
{
    switch (mBehaviorState)
    {
        case BehaviorState::Idle:
            if (mRigidBodyComponent->GetVelocity().Length() > 0.1f)
                mBehaviorState = BehaviorState::Moving;
            break;
        
        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetVelocity().Length() < 0.1f)
                mBehaviorState = BehaviorState::Idle;
            break;
        
        default:
            mBehaviorState = BehaviorState::Idle;           
            break;
    }
}

void Star::OnUpdate(float deltaTime)
{
    ManageState();

    if (mBehaviorState == BehaviorState::Moving)
    {
        SetRotation(GetRotation() + deltaTime*300);
    }
}