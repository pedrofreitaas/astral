//
// Created by Lucas N. Ferreira on 08/09/23.
//

#include <SDL.h>
#include "../actors/Actor.h"
#include "../core/Game.h"
#include "RigidBodyComponent.h"
#include "collider/AABBColliderComponent.h"

const float MAX_SPEED_X = 750.0f;
const float MAX_SPEED_Y = 750.0f;

RigidBodyComponent::RigidBodyComponent(class Actor* owner, float mass, float friction, bool applyGravity, int updateOrder)
        :Component(owner, updateOrder)
        ,mMass(mass)
        ,mApplyGravity(applyGravity)
        ,mApplyFriction(true)
        ,mFrictionCoefficient(friction)
        ,mVelocity(Vector2::Zero)
        ,mAcceleration(Vector2::Zero)
        ,mIsOnGround(false)
{
    if (mMass <= 0.f) {
        throw std::invalid_argument("Mass must be greater than zero");
    }
}

// Force changes acceleration
// Continues application over time
void RigidBodyComponent::ApplyForce(const Vector2 &force) {
    mAcceleration += force * (1.f/mMass);
}

// Impulse changes velocity directly
// Should be applied in a single frame
void RigidBodyComponent::ApplyImpulse(const Vector2 &impulse) {
    mVelocity += impulse * (1.f/mMass);
}

void RigidBodyComponent::Update(float deltaTime)
{
    bool sceneAppliesGravity = mOwner->GetGame()->GetApplyGravityScene();
    bool applyGravity = mApplyGravity && sceneAppliesGravity;
    
    if(applyGravity) ApplyForce(Vector2(0.f, GRAVITY));

    if (mApplyFriction && mIsOnGround) ApplyForce(Vector2(-mFrictionCoefficient * mVelocity));

    // Euler Integration
    mVelocity += mAcceleration * deltaTime;

    mVelocity.x = Math::Clamp<float>(mVelocity.x, -MAX_SPEED_X, MAX_SPEED_X);
    mVelocity.y = Math::Clamp<float>(mVelocity.y, -MAX_SPEED_Y, MAX_SPEED_Y);

    if (Math::NearZero(mVelocity.x, .01f)) {
        mVelocity.x = 0.f;
    }

    if (Math::NearZero(mVelocity.y, .01f)) {
        mVelocity.y = 0.f;
    }

    auto collider = mOwner->GetComponent<AABBColliderComponent>();

    mOwner->SetPosition(Vector2(mOwner->GetPosition().x + mVelocity.x * deltaTime,
                                mOwner->GetPosition().y));

    if (collider) {
        collider->DetectHorizontalCollision(this);
    }

    mOwner->SetPosition(Vector2(mOwner->GetPosition().x,
                                mOwner->GetPosition().y + mVelocity.y * deltaTime));

    if (collider) {
        float t = collider->DetectVerticalCollision(this);
        mIsOnGround = applyGravity ? (t > 0.0f) : true;
    }

    mAcceleration = Vector2::Zero;
}

float RigidBodyComponent::GetJumpImpulseY(float totalBlocks) {
    const float height = totalBlocks * Game::TILE_SIZE;
    const float v0 = std::sqrt(2.0f * GRAVITY * height);

    return -mMass * v0;
}