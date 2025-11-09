#include "Projectile.h"
#include "Zoe.h"
#include "../components/RigidBodyComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/TimerComponent.h"

Projectile::Projectile(
    class Game* game, Vector2 position, 
    Vector2 direction, float speed
): Actor(game), mDirection(direction), mSpeed(speed), mKnockbackIntensity(10.f) {
    SetPosition(position);

    mTimerComponent = new TimerComponent(this);

    mTimerComponent->AddTimer(MAX_DIE_TIME, [this]() {
        Kill();
    });
}

void Projectile::OnUpdate(float deltaTime) {
    ManageAnimations();

    if (mBehaviorState == BehaviorState::Dying)
        return;
    
    Vector2 movement = mDirection * mSpeed * deltaTime;
    mRigidBodyComponent->ApplyForce(movement);
}

void Projectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;

    // projectile can die immediately on collision, so the take damage logic is on the projectile
    if (other->GetLayer() == ColliderLayer::Player)
    {
        auto player = dynamic_cast<Zoe*>(GetGame()->GetZoe());
        if (player)
        {
            Vector2 mSpeedDir = mRigidBodyComponent->GetAppliedForce();
            player->TakeDamage(mSpeedDir * mKnockbackIntensity);
        }
    }
    Kill();
}

void Projectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;

    // projectile can die immediately on collision, so the take damage logic is on the projectile
    if (other->GetLayer() == ColliderLayer::Player)
    {
        auto player = dynamic_cast<Zoe*>(GetGame()->GetZoe());
        if (player)
        {
            Vector2 mSpeedDir = mRigidBodyComponent->GetAppliedForce();
            player->TakeDamage(mSpeedDir * mKnockbackIntensity);
        }
    }
    Kill();
}

void Projectile::Kill() { 
    mBehaviorState = BehaviorState::Dying; 
}
