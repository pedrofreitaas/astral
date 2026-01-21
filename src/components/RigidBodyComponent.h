//
// Created by Lucas N. Ferreira on 08/09/23.
//

#pragma once
#include "Component.h"
#include "../libs/Math.h"

const float GRAVITY = 980.0f;

class RigidBodyComponent : public Component
{
public:
    // Lower update order to update first
    RigidBodyComponent(class Actor* owner, float mass = 1.0f, float friction = 0.0f,
                        bool applyGravity = true, int updateOrder = 10);

    void Update(float deltaTime) override;

    const Vector2& GetVelocity() const { return mVelocity; }
    void ResetVelocity() { mVelocity = Vector2(0,0); }
    void ResetVelocityX() { mVelocity.x = 0; }
    void ResetVelocityY() { mVelocity.y = 0; }

    const Vector2& GetAcceleration() const { return mAcceleration; }
    void ResetAcceleration() { mAcceleration = Vector2(0,0); }

    void SetApplyGravity(const bool applyGravity) { mApplyGravity = applyGravity; }
    void SetApplyFriction(const bool applyFriction) { mApplyFriction = applyFriction;  }

    void ApplyForce(const Vector2 &force);
    void ApplyImpulse(const Vector2 &impulse);

    bool GetOnGround() { return mIsOnGround; }

    float GetJumpImpulseY(float totalBlocks);

    int SpeedHDir() {
        if (mVelocity.x == 0) return 0;
        if (mVelocity.x > 0) return 1;
        return -1;
    }

    bool GetApplyGravity() const { return mApplyGravity; }
    Vector2 GetAppliedForce() const { return mAcceleration * mMass; }

    void SetGravityScale(float scale) { 
        if (scale < 0.f) scale = 0.f;
        if (scale > 1.f) scale = 1.f;
        
        mGravityScale = scale; 
    }

private:
    bool mApplyGravity;
    bool mApplyFriction;

    // Physical properties
    float mFrictionCoefficient;
    float mMass;
    bool mIsOnGround;
    float mGravityScale;

    Vector2 mVelocity;
    Vector2 mAcceleration;
};
