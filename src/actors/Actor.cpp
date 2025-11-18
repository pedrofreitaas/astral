// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------
#include "Actor.h"
#include "../core/Game.h"
#include "../components/Component.h"
#include <algorithm>

Actor::Actor(Game* game, int lives)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(1.0f)
        , mRotation(0.0f)
        , mGame(game)
        , mIsOnGround(false)
        , mBehaviorState(BehaviorState::Idle)
        , mLives(lives)
        , mInvincible(false)
{
    mGame->AddActor(this);
}

Actor::~Actor()
{
    mGame->RemoveActor(this);

    for(auto component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::SetPosition(const Vector2& pos)
{
    mPosition = pos;
    mGame->Reinsert(this);
}

void Actor::SetCenter(const Vector2& pos)
{
    mPosition = pos - GetCenter()*.5f;
    mGame->Reinsert(this);
}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        for (auto comp : mComponents)
        {
            if(comp->IsEnabled())
            {
                comp->Update(deltaTime);
            }
        }

        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{

}

void Actor::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {

}

void Actor::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {

}

void Actor::Kill()
{

}

void Actor::ProcessInput(const Uint8* keyState)
{
    if (mState == ActorState::Active)
    {
        for (auto comp : mComponents)
        {
            comp->ProcessInput(keyState);
        }

        OnProcessInput(keyState);
    }
}

void Actor::HandleKeyPress(const int key, const bool isPressed)
{
    if (mState == ActorState::Active)
    {
        for (auto comp : mComponents)
        {
            comp->HandleKeyPress(key, isPressed);
        }

        // Call actor-specific key press handling
        OnHandleKeyPress(key, isPressed);
    }

}

void Actor::OnProcessInput(const Uint8* keyState)
{

}

void Actor::OnHandleKeyPress(const int key, const bool isPressed)
{

}

void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);
    std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
        return a->GetUpdateOrder() < b->GetUpdateOrder();
    });
}

bool Actor::IsVisibleOnCamera() const
{
    // Get the camera's position and dimensions
    Vector2 cameraPosition = mGame->GetCameraPos();
    float screenWidth = mGame->GetWindowWidth();
    float screenHeight = mGame->GetWindowHeight();

    // Check if the actor's position is within the camera's view
    return (mPosition.x >= cameraPosition.x && mPosition.x <= cameraPosition.x + screenWidth &&
            mPosition.y >= cameraPosition.y && mPosition.y <= cameraPosition.y + screenHeight);
}

Vector2 Actor::GetCenter() const
{
    auto collider = GetComponent<AABBColliderComponent>();
    
    if (collider)
        return collider->GetCenter();
    else
        return mPosition;
}

void Actor::LogState()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        SDL_Log("BehaviorState: Idle");
        break;
    case BehaviorState::Moving:
        SDL_Log("BehaviorState: Moving");
        break;
    case BehaviorState::TakingDamage:
        SDL_Log("BehaviorState: TakingDamage");
        break;
    case BehaviorState::Dying:
        SDL_Log("BehaviorState: Dying");
        break;
    case BehaviorState::Jumping:
        SDL_Log("BehaviorState: Jumping");
        break;
    default:
        SDL_Log("BehaviorState: Other");
        break;
    }
}

void Actor::TakeDamage(const Vector2 &knockback)
{
    if (mBehaviorState == BehaviorState::Dying || mInvincible)
        return;

    mLives--;

    auto rigidBody = GetComponent<RigidBodyComponent>();
    if (rigidBody)
    {
        rigidBody->ApplyForce(knockback);
    }

    mBehaviorState = BehaviorState::TakingDamage;
    mInvincible = true;
}