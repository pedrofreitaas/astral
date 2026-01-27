// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <vector>
#include <SDL_stdinc.h>
#include "../libs/Math.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/TimerComponent.h"

enum class ActorState
{
    Active,
    Paused,
    Destroy
};

enum class BehaviorState // For AI behaviors/animations
{
    Asleep,
    Waking,
    Idle,
    Moving,
    Charging,
    Attacking, 
    AerialAttacking,
    Dodging,
    Stunned,
    Jumping,
    Falling,
    Dying,
    Provoking,
    Fleeing,
    Wandering,
    TakingDamage,
    Clinging
};

class Actor
{
public:
    Actor(class Game* game, int lives=3, bool mustAlwaysUpdate = false);
    virtual ~Actor();

    // it's actor's responsibility to become not invicible again
    virtual void TakeDamage(const Vector2 &knockback = Vector2(0.f, 0.f));

    void SetLifes(int lives) { mLifes = lives; }
    int GetLifes() { return mLifes; }

    // Reinsert function called from Game (not overridable)
    void Update(float deltaTime);
    // ProcessInput function called from Game (not overridable)
    void ProcessInput(const Uint8* keyState);
    // HandleKeyPress function called from Game (not overridable)
    void HandleKeyPress(const int key, const bool isPressed);

    // Position getter/setter
    const Vector2& GetPosition() const { return mPosition; }
    void SetPosition(const Vector2& pos);
    void SetCenter(const Vector2& pos);

    Vector2 GetForward() const { return Vector2(Math::Cos(mRotation), -Math::Sin(mRotation)); }

    // Scale getter/setter
    float GetScale() const { return mScale; }
    void SetScale(float scale) { mScale = scale; }

    // Rotation getter/setter
    float GetRotation() const { return mRotation; }
    void SetRotation(float rotation) { mRotation = rotation; }

    // State getter/setter
    ActorState GetState() const { return mState; }
    void SetState(ActorState state) { mState = state; }

    // Game getter
    class Game* GetGame() { return mGame; }

    // Returns component of type T, or null if doesn't exist
    template <typename T>
    T* GetComponent() const
    {
        for (auto c : mComponents)
        {
            T* t = dynamic_cast<T*>(c);
            if (t != nullptr)
            {
                return t;
            }
        }

        return nullptr;
    }

    template <typename T>
    std::vector<T*> GetComponents() const
    {
        std::vector<T*> result;
        for (auto c : mComponents)
        {
            if (auto casted = dynamic_cast<T*>(c))
                result.emplace_back(casted);
        }
        return result;
    }

    // Game specific
    void SetOnGround() { mIsOnGround = true; };
    void SetOffGround() { mIsOnGround = false; };
    bool IsOnGround() const { return mIsOnGround; };
    bool IsVisibleOnCamera() const;

    // Any actor-specific collision code (overridable)
    virtual void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other);
    virtual void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other);
    virtual void OnCollision() {};
    virtual void Kill();
    
    Vector2 GetCenter() const;
    Vector2 GetHalfSize() const;

    BehaviorState GetBehaviorState() const { return mBehaviorState; }

    void LogState();

    int HowManyComponents() const { return static_cast<int>(mComponents.size()); }

    const std::string& GetType() const { return mType; }

protected:
    class Game* mGame;

    // Any actor-specific update code (overridable)
    virtual void OnUpdate(float deltaTime);
    virtual void OnProcessInput(const Uint8* keyState);
    virtual void OnHandleKeyPress(const int key, const bool isPressed);

    void TakeSpikeHit(const Vector2 &SpikeBaseCenter);
    void TakeSpearHit(const Vector2 &SpearTipCenter);
    void TakeShurikenHit(const Vector2 &ShurikenCenter);

    std::string mType;

    // Actor's state
    ActorState mState;
    BehaviorState mBehaviorState;

    // Transform
    Vector2 mPosition;
    float mScale;
    float mRotation;
    float mFreezingCount;
    bool mIsFrozen;

    // Components
    std::vector<class Component*> mComponents;

    // Game specific
    bool mIsOnGround;
    int mLifes;
    bool mInvincible;

    class TimerComponent* mTimerComponent;

    void UpdateFreezing();
    void IncreaseFreezing(float modifier=1.f);
    virtual void Freeze();
    virtual void StopFreeze();

private:
    friend class Component;

    // Adds component to Actor (this is automatically called
    // in the component constructor)
    void AddComponent(class Component* c);
};