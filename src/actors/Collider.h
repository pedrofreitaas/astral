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
    Both
};

class Collider : public Actor
{
public:
    Collider(
        Game *game,
        const Vector2 &position,
        const Vector2 &size,
        std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> dismissCallback, 
        DismissOn dismissOn,
        ColliderLayer layer,
        std::vector<ColliderLayer> ignoredLayers = {},
        float timeToDismiss = 0.f);

    void Dismiss();
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

private:
    std::function<void(bool collided, const float minOverlap, AABBColliderComponent *other)> mDismissCallback;
    DismissOn mDismissOn;
    float mTimeToDismiss;

    class AABBColliderComponent* mColliderComponent;
    class TimerComponent* mTimerComponent;
};