//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include "Component.h"
#include "../libs/Math.h"

class RigidBodyComponent : public Component
{
public:
    // Lower update order to update first
    RigidBodyComponent(class Actor* owner, float mass = 1.0f, float friction = 0.0f,
                        bool applyGravity = true, int updateOrder = 10);

    void Update(float deltaTime) override;

    const Vector2& GetVelocity() const { return mVelocity; }
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }
    void SumVelocity(const Vector2& velocity) { mVelocity += velocity; }

    const Vector2& GetAcceleration() const { return mAcceleration; }
    void SetAcceleration(const Vector2& acceleration) { mAcceleration = acceleration; }

    void SetApplyGravity(const bool applyGravity) { mApplyGravity = applyGravity; }
    void SetApplyFriction(const bool applyFriction) { mApplyFriction = applyFriction;  }

    void ApplyForce(const Vector2 &force);
    bool GetOnGround() { return mIsOnGround; }

    float GetVerticalForce(float totalBlocks);

    int SpeedHDir() {
        if (mVelocity.x == 0) return 0;
        if (mVelocity.x > 0) return 1;
        return -1;
    }

    bool GetApplyGravity() const { return mApplyGravity; }

    Vector2 GetAppliedForce() const { return mAcceleration * mMass; }

private:
    bool mApplyGravity;
    bool mApplyFriction;

    // Physical properties
    float mFrictionCoefficient;
    float mMass;
    bool mIsOnGround;

    Vector2 mVelocity;
    Vector2 mAcceleration, mNotAppliedAcceleration;

    bool hasProcessedAppliedForceThisFrame;
};
