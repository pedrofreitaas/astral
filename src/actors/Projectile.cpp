#include "Projectile.h"
#include "Zoe.h"
#include "../components/RigidBodyComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/TimerComponent.h"

Projectile::Projectile(
    class Game* game, Vector2 position, 
    Vector2 target, float speed,
    Actor* shooter,
    float mDieTime
): Actor(game), mTarget(target), mSpeed(speed), 
   mKnockbackIntensity(10.f), mDirection(Vector2::Zero), mShooter(shooter), mDieTime(mDieTime)
{
    SetPosition(position);

    mTimerComponent = new TimerComponent(this);

    mTimerComponent->AddTimer(mDieTime, [this]() {
        Kill();
    });
}

void Projectile::OnUpdate(float deltaTime) {
    ManageAnimations();

    if (mBehaviorState == BehaviorState::Dying)
    {
        mRigidBodyComponent->ResetVelocity();
        return;
    }
    
    Vector2 movForce = mDirection * mSpeed * deltaTime;
    mRigidBodyComponent->ApplyForce(movForce);
}

void Projectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;
    if (other->GetLayer() == mShooter->GetComponent<AABBColliderComponent>()->GetLayer()) return;

    Kill();
}

void Projectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;
    if (other->GetLayer() == mShooter->GetComponent<AABBColliderComponent>()->GetLayer()) return;

    Kill();
}

void Projectile::Kill() { 
    mBehaviorState = BehaviorState::Dying; 
}
