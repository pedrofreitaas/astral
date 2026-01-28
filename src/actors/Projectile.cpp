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
): Actor(game), mKnockbackIntensity(10.f), mShooter(shooter), mDieTime(mDieTime)
{
    SetPosition(position);

    mTimerComponent = new TimerComponent(this);

    mTimerComponent->AddTimer(mDieTime, [this]() {
        Kill();
    });
}

void Projectile::Fire(const Vector2& direction, float speed) {
    Vector2 normalizedDirection = Vector2::Normalize(direction);
    mRigidBodyComponent->ApplyImpulse(normalizedDirection * speed);
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
