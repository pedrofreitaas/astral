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
{

}

void RigidBodyComponent::ApplyForce(const Vector2 &force) {
    mAcceleration += force * (1.f/mMass);
}

void RigidBodyComponent::Update(float deltaTime)
{
    // Apply gravity acceleration
    if(mApplyGravity) SDL_Log("gravity w.i.p.");

    // Apply friction
    if (mApplyFriction) ApplyForce(-mFrictionCoefficient * mVelocity);

    // Euler Integration
    mVelocity += mAcceleration * deltaTime;

    // Clamp velocity
    mVelocity.x = Math::Clamp<float>(mVelocity.x, -MAX_SPEED_X, MAX_SPEED_X);
    mVelocity.y = Math::Clamp<float>(mVelocity.y, -MAX_SPEED_Y, MAX_SPEED_Y);

    if (Math::NearZero(mVelocity.x, 1.0f)) {
        mVelocity.x = 0.f;
    }

    if (Math::NearZero(mVelocity.y, 1.0f)) {
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
        collider->DetectVertialCollision(this);
    }

    mAcceleration.Set(0.f, 0.f);
}

