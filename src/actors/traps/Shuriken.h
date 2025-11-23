#pragma once

#include <SDL.h>
#include "../Actor.h"
#include "../../components/TimerComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../Collider.h"

class Shuriken : public Actor
{
public:
    const float TRIGGER_COOLDOWN = 2.f;

    explicit Shuriken(Game* game, const Vector2& position);

    void ManageState();
    void ManageAnimations();
    void OnUpdate(float deltaTime) override;

private:
    TimerComponent *mTimerComponent;
    AABBColliderComponent *mColliderComponent;
    DrawAnimatedComponent *mDrawComponent;
    RigidBodyComponent *mRigidBodyComponent; //just to check the collision
};