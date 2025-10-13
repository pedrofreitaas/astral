#pragma once
#include <SDL.h>
#include "../../actors/Actor.h"
#include "../Component.h"
#include "../../libs/Math.h"
#include "../RigidBodyComponent.h"
#include "../collider/AABBColliderComponent.h"
#include "../../libs/Math.h"

enum class MovementState
{
    Wandering,
    Jumping,
    FollowingPath
};

class AIMovementComponent : public Component
{
public:
    AIMovementComponent(class Actor* owner, float fowardSpeed, float craziness=.05f);
    ~AIMovementComponent() override = default;

    void SetFowardSpeed(float speed) { mFowardSpeed = speed; }
    float GetFowardSpeed() const { return mFowardSpeed; }

private:
    void Sense(float deltaTime);
    void Plan(float deltaTime);
    void Act(float deltaTime);

    bool CrazyDecision();
    bool CrazyDecision(float modifier);

    void Jump();

    void Update(float deltaTime) override;

    RigidBodyComponent* GetOwnerRigidBody() {
        return mOwner->GetComponent<RigidBodyComponent>();
    }

    AABBColliderComponent* GetOwnerCollider() {
        return mOwner->GetComponent<AABBColliderComponent>();
    }

    MovementState mMovementState;
    int mJumpForceInBlocks;
    float mInteligence, mCraziness;
    float mFowardSpeed;
};
