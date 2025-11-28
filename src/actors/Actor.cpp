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
#include "./Collider.h"
#include "./traps/Spikes.h"
#include "./traps/Spear.h"
#include "./traps/Shuriken.h"
#include "../libs/Math.h"

Actor::Actor(Game* game, int lives, bool mustAlwaysUpdate)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(1.0f)
        , mRotation(0.0f)
        , mGame(game)
        , mIsOnGround(false)
        , mBehaviorState(BehaviorState::Idle)
        , mLifes(lives)
        , mInvincible(false)
{
    mGame->AddActor(this);
    
    SetLifes(lives);

    if (mustAlwaysUpdate) {
        mGame->AddMustAlwaysUpdateActor(this);
    }
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

void Actor::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (other->GetLayer() == ColliderLayer::Spikes)
    {
        Collider *spikeCollider = static_cast<Collider*>(other->GetOwner());
        Spikes *spikes = static_cast<Spikes*>(spikeCollider->GetOwnerActor());
        TakeSpikeHit(spikes->GetBaseCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::SpearTip)
    {
        Collider *spearTipCollider = static_cast<Collider*>(other->GetOwner());
        Spear *spear = static_cast<Spear*>(spearTipCollider->GetOwnerActor());
        TakeSpearHit(spear->GetTipCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::Shuriken) //shuriken doesnt use intermediate collider
    {
        Shuriken *shuriken = static_cast<Shuriken*>(other->GetOwner());
        TakeShurikenHit(shuriken->GetCenter());
        return;
    }
}

void Actor::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (other->GetLayer() == ColliderLayer::Spikes)
    {
        Collider *spikeCollider = static_cast<Collider*>(other->GetOwner());
        Spikes *spikes = static_cast<Spikes*>(spikeCollider->GetOwnerActor());
        TakeSpikeHit(spikes->GetBaseCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::SpearTip)
    {
        Collider *spearTipCollider = static_cast<Collider*>(other->GetOwner());
        Spear *spear = static_cast<Spear*>(spearTipCollider->GetOwnerActor());
        TakeSpearHit(spear->GetTipCenter());
        return;
    }

    if (other->GetLayer() == ColliderLayer::Shuriken) //shuriken doesnt use intermediate collider
    {
        Shuriken *shuriken = static_cast<Shuriken*>(other->GetOwner());
        TakeShurikenHit(shuriken->GetCenter());
        return;
    }
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

    mLifes--;

    auto rigidBody = GetComponent<RigidBodyComponent>();
    if (rigidBody)
    {
        rigidBody->SetVelocity(Vector2(0.f,0.f));
        rigidBody->ApplyForce(knockback);
    }

    if (mLifes <= 0)
    {
        mBehaviorState = BehaviorState::Dying;
        return;
    }

    mBehaviorState = BehaviorState::TakingDamage;
    mInvincible = true;
}

Vector2 Actor::GetHalfSize() const
{
    return GetCenter() - GetPosition();
}

void Actor::TakeSpikeHit(const Vector2 &SpikeBaseCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - SpikeBaseCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Actor::SPIKE_KNOCKBACK_FORCE);
}

void Actor::TakeSpearHit(const Vector2 &SpearTipCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - SpearTipCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Actor::SPEAR_KNOCKBACK_FORCE);
}

void Actor::TakeShurikenHit(const Vector2 &ShurikenCenter)
{
    float knockbackX = Math::Sign(GetCenter().x - ShurikenCenter.x);

    Vector2 knockback = Vector2(knockbackX, -1.f);
    knockback.Normalize();

    TakeDamage(knockback * Actor::SHURIKEN_KNOCKBACK_FORCE);
}