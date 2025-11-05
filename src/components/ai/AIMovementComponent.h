#pragma once
#include <SDL.h>
#include "../../actors/Actor.h"
#include "../Component.h"
#include "../../libs/Math.h"
#include "../RigidBodyComponent.h"
#include "../collider/AABBColliderComponent.h"
#include "../../libs/Math.h"

enum class TypeOfMovement
{
    Walker,
    Flier
};

enum class MovementState
{
    Wandering,
    Jumping,
    FollowingPath,
    FollowingPathJumping
};

// actor if this component should apply any sort of force, or movement logic directly.
// it must use this as a middleware.

class AIMovementComponent : public Component
{
public:
    AIMovementComponent(
        class Actor* owner, float fowardSpeed, 
        int jumpForceInBlocks, TypeOfMovement typeOfMovement=TypeOfMovement::Walker,
        float pathTolerance=10.f, float craziness=.05f);
    ~AIMovementComponent() override = default;

    void SetFowardSpeed(float speed) { mFowardSpeed = speed; }
    float GetFowardSpeed() const { return mFowardSpeed; }
    void SeekPlayer();

    MovementState GetMovementState() const { return mMovementState; }
    void SetMovementState(MovementState state) { 
        mPreviousMovementState = mMovementState; 
        mMovementState = state; 
        LogState();
    }
    void LogState();

    std::vector<SDL_Rect> GetPath() const { return mPath; }

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other);
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other);
    void ApplyForce(const Vector2 &force);

    bool CrazyDecision();
    bool CrazyDecision(float modifier);

private:
    void Sense(float deltaTime);
    void Plan(float deltaTime);
    void Act(float deltaTime);

    void Jump(bool isFollowingPath = false);

    void Update(float deltaTime) override;

    RigidBodyComponent* GetOwnerRigidBody() {
        return mOwner->GetComponent<RigidBodyComponent>();
    }

    AABBColliderComponent* GetOwnerCollider() {
        return mOwner->GetComponent<AABBColliderComponent>();
    }

    int GetTargetIndex();

    TypeOfMovement mTypeOfMovement;
    MovementState mMovementState;
    MovementState mPreviousMovementState;
    int mJumpForceInBlocks;
    float mInteligence, mCraziness;
    float mFowardSpeed;

    std::vector<SDL_Rect> mPath;
    float mPathTimer;
    float mPathTolerance;

    void FollowPathFlier();
    void FollowPathWalker();
};
