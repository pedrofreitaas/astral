#pragma once

#include <SDL.h>
#include "../Actor.h"
#include "../../components/TimerComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../Collider.h"

class Spikes : public Actor
{
public:
    const float TRIGGER_COOLDOWN = .75f;

    explicit Spikes(Game* game, const Vector2& position);

    void ManageState();
    void AnimationEndCallback(std::string animationName);
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;
    void Trigger();
    Vector2 GetBaseCenter() const;

private:
    TimerComponent *mTimerComponent;
    AABBColliderComponent *mColliderComponent;
    DrawAnimatedComponent *mDrawComponent;
    RigidBodyComponent *mRigidBodyComponent; //just to check the collision

    Collider *mSpikeCollider;
};