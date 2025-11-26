#pragma once

#include <SDL.h>
#include "../Actor.h"
#include "../../components/TimerComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../Collider.h"

class Spear : public Actor
{
public:
    const float TRIGGER_COOLDOWN = .5f;

    explicit Spear(Game* game, const Vector2& position);

    void ManageState();
    void AnimationEndCallback(std::string animationName);
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;
    void Trigger();
    Vector2 GetTipCenter() const;

private:
    TimerComponent *mTimerComponent;
    AABBColliderComponent *mColliderComponent;
    DrawAnimatedComponent *mDrawComponent;
    RigidBodyComponent *mRigidBodyComponent; //just to check the collision

    Collider *mTipCollider;
};