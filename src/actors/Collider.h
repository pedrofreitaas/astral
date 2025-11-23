#pragma once

#include <functional>
#include <utility>
#include <string>
#include "Actor.h"
#include "../core/Game.h"
#include "../components/TimerComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../libs/Math.h"

enum class DismissOn {
    Collision,
    Time,
    Both,
    None
};

class Collider : public Actor
{
public:
    // isTangible false wont solve collision 
    Collider(
        Game *game,
        Actor *owner,
        const Vector2 &position,
        const Vector2 &size,
        std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> collideCallback, 
        DismissOn dismissOn,
        ColliderLayer layer,
        std::vector<ColliderLayer> ignoredLayers = {},
        float timeToDismiss = 0.f,
        std::function<void()> dismissCallback = nullptr,
        bool isTangible = true);

    void Dismiss();
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void SetEnabled(bool enabled);
    Actor* GetOwnerActor() const { return mOwner; }

private:
    std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> mCollideCallback;
    std::function<void()> mDismissCallback;
    DismissOn mDismissOn;
    float mTimeToDismiss;

    class AABBColliderComponent* mColliderComponent;
    class TimerComponent* mTimerComponent;
    Actor* mOwner;
};