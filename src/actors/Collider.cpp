#include "Collider.h"

Collider::Collider(
    Game *game,
    const Vector2 &position,
    const Vector2 &size,
    std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> dismissCallback, 
    DismissOn dismissOn,
    ColliderLayer layer,
    std::vector<ColliderLayer> ignoredLayers,
    float timeToDismiss
): Actor(game), 
   mDismissCallback(std::move(dismissCallback)),
   mDismissOn(dismissOn), 
   mTimeToDismiss(timeToDismiss)
{
    mColliderComponent = new AABBColliderComponent(
        this,
        0.f, 0.f,
        size.x, size.y,
        layer,
        true,
        10);
    
    mColliderComponent->IgnoreLayers(ignoredLayers);
    mColliderComponent->SetEnabled(true);

    mTimerComponent = new TimerComponent(this);

    if (mDismissOn == DismissOn::Time || mDismissOn == DismissOn::Both)
    {
        mTimerComponent->AddTimer(mTimeToDismiss, [this]() {
            if (mDismissCallback) {
                mDismissCallback(false, 0.f, nullptr);
            }
            SetState(ActorState::Destroy);
        });
    }

    SetPosition(position);
}

void Collider::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (mDismissCallback) {
        mDismissCallback(true, minOverlap, other);
    }
    if (mDismissOn == DismissOn::Collision || mDismissOn == DismissOn::Both) {
        SetState(ActorState::Destroy);
    }
}

void Collider::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (mDismissCallback) {
        mDismissCallback(true, minOverlap, other);
    }
    if (mDismissOn == DismissOn::Collision || mDismissOn == DismissOn::Both) {
        SetState(ActorState::Destroy);
    }
}

void Collider::Dismiss()
{
    SetState(ActorState::Destroy);
}