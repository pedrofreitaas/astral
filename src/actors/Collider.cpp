#include "Collider.h"

Collider::Collider(
    Game *game,
    Actor *owner,
    const Vector2 &position,
    const Vector2 &size,
    std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> collideCallback, 
    DismissOn dismissOn,
    ColliderLayer layer,
    std::vector<ColliderLayer> ignoredLayers,
    float timeToDismiss,
    std::function<void()> dismissCallback,
    bool isTangible
): Actor(game), 
   mOwner(owner),
   mCollideCallback(std::move(collideCallback)),
   mDismissOn(dismissOn), 
   mTimeToDismiss(timeToDismiss),
   mDismissCallback(std::move(dismissCallback))
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
    mColliderComponent->SetIsTangible(isTangible);

    mTimerComponent = new TimerComponent(this);

    if (mDismissOn == DismissOn::Time || mDismissOn == DismissOn::Both)
    {
        mTimerComponent->AddTimer(mTimeToDismiss, [this]() {
            if (mDismissCallback) {
                mDismissCallback();
            }
            SetState(ActorState::Destroy);
        });
    }

    SetPosition(position);
}

void Collider::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (mCollideCallback) {
        mCollideCallback(true, minOverlap, other);
    }

    if (mDismissOn == DismissOn::Collision || mDismissOn == DismissOn::Both) {
        SetState(ActorState::Destroy);
        if (mDismissCallback) {
            mDismissCallback();
        }
    }
}

void Collider::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) 
{
    if (mCollideCallback) {
        mCollideCallback(true, minOverlap, other);
    }

    if (mDismissOn == DismissOn::Collision || mDismissOn == DismissOn::Both) {
        SetState(ActorState::Destroy);
        if (mDismissCallback) {
            mDismissCallback();
        }
    }
}

void Collider::Dismiss()
{
    SetState(ActorState::Destroy);
}

void Collider::SetEnabled(bool enabled)
{
    mColliderComponent->SetEnabled(enabled);
}

bool Collider::IsEnabled() const
{
    return mColliderComponent->IsEnabled();
}