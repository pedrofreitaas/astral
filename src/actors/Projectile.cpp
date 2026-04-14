#include "Projectile.h"
#include "Zoe.h"
#include "../components/RigidBodyComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/TimerComponent.h"

Projectile::Projectile(
    class Game* game, 
    Vector2 position,
    Actor* shooter,
    float mDieTime
): Actor(game), mKnockbackIntensity(10.f), mShooter(shooter), mDieTime(mDieTime), 
   mLastFireDirection(Vector2::Zero), mDieTimer(nullptr)
{
    SetPosition(position);

    mTimerComponent = new TimerComponent(this);

    mDieTimer = mTimerComponent->AddTimer(mDieTime, [this]() {
        Kill();
    });
}

void Projectile::Fire(const Vector2& dirNormalized, float speed) {
    mRigidBodyComponent->ApplyImpulse(dirNormalized * speed);
    mLastFireDirection = dirNormalized;
}

void Projectile::OnUpdate(float deltaTime) {
    ManageAnimations();

    if (mBehaviorState == BehaviorState::Dying)
    {
        mRigidBodyComponent->ResetVelocity();
        return;
    }
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
