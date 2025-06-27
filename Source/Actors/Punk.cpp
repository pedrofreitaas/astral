//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Punk.h"
#include "Block.h"
#include "../Game.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

float DEATH_TIME = .35f;

Punk::Punk(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsRunning(false)
        , mIsDying(false)
        , mDeathTimer(0.0f)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);

    mColliderComponent = new AABBColliderComponent(this, 14, 20, 18, 28,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this, 
                                               "../Assets/Sprites/Punk/texture.png",
                                               "../Assets/Sprites/Punk/texture.json");

    mDrawComponent->AddAnimation("dying", {13,14,15,16,17,18});
    mDrawComponent->AddAnimation("idle", {0,1,2,3});
    mDrawComponent->AddAnimation("run", {4,5,6,7,8,9});
    mDrawComponent->AddAnimation("jump", {10,11,12,13});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);
}

void Punk::OnProcessInput(const uint8_t* state)
{
    if (mIsDying) return;

    mIsRunning = false;

    if (state[SDL_SCANCODE_D]) {
        mRigidBodyComponent->ApplyForce(Vector2(mForwardSpeed, 0.0f));
        SetRotation(0.0f);
        mIsRunning = true;
    }
    if (state[SDL_SCANCODE_A]) {
        mRigidBodyComponent->ApplyForce(Vector2(-mForwardSpeed, 0.0f));
        SetRotation(Math::Pi);
        mIsRunning = true;
    }
    
    if (state[SDL_SCANCODE_W]) {
        mRigidBodyComponent->ApplyForce(Vector2(0.0f, -mForwardSpeed));
        mIsRunning = true;
    }

    if (state[SDL_SCANCODE_S]) {
        mRigidBodyComponent->ApplyForce(Vector2(0.0f, mForwardSpeed));
        mIsRunning = true;
    }
}

void Punk::MaintainInbound() {
    Vector2 cameraPos = GetGame()->GetCameraPos();
    Vector2 getUpperLeftBorder = mColliderComponent->GetMin();
    Vector2 getBottomRightBorder = mColliderComponent->GetMax();
    Vector2 offset = mColliderComponent->GetOffset();
    int mWidth = mColliderComponent->GetWidth();
    int mHeight = mColliderComponent->GetHeight();
    int maxXBoundary = cameraPos.x + GetGame()->GetWindowWidth();
    int maxYBoundary = cameraPos.y + GetGame()->GetWindowHeight();
    
    if (getUpperLeftBorder.x < 0) {
        SetPosition(Vector2(-offset.x, GetPosition().y));
    }
    else if (getBottomRightBorder.x > maxXBoundary) {
        SetPosition(Vector2(maxXBoundary-mWidth-offset.x, GetPosition().y));
    }

    if (getUpperLeftBorder.y < 0) {
        SetPosition(Vector2(GetPosition().x, -offset.y));
    }
    else if (getBottomRightBorder.y > maxYBoundary) {
        SetPosition(Vector2(GetPosition().x, maxYBoundary-mHeight-offset.y));
    }
}

void Punk::OnUpdate(float deltaTime)
{
    MaintainInbound();

    ManageAnimations();

    if (!mIsDying) return;

    mDeathTimer -= deltaTime;

    if (mDeathTimer > 0.0f) return; 
    
    mColliderComponent->SetEnabled(false);
    mRigidBodyComponent->SetEnabled(false);
    mDrawComponent->SetEnabled(false);
    
    mGame->Quit();
}

void Punk::ManageAnimations()
{
    if (mIsDying) {
        mDrawComponent->SetAnimation("dying");
    }
    else if (mIsRunning) {
        mDrawComponent->SetAnimation("run");
    }
    else if (!mIsRunning) {
        mDrawComponent->SetAnimation("idle");
    }
}

void Punk::Kill()
{
    if (mIsDying) return;
    
    mIsDying = true;
    mDeathTimer = DEATH_TIME;
    mRigidBodyComponent->SetVelocity(
        Vector2(0.0f, mJumpSpeed/3)
    );
    mDrawComponent->SetAnimation("dying");
}

void Punk::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mIsDying) return;

    if (other->GetLayer() == ColliderLayer::Enemy) {
        Kill();
    }
}

void Punk::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mIsDying) return;
    
    if (other->GetLayer() == ColliderLayer::Enemy) {
        other->GetOwner()->Kill();
        mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed / 2));
    }

    else if (other->GetLayer() == ColliderLayer::Bricks && minOverlap < 0) {
        Block *block = static_cast<Block*>(other->GetOwner());
        block->OnColision();
    }
}
