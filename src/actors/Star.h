#pragma once
#include "Actor.h"
#include <SDL.h>

class Star : public Actor
{
public:
    explicit Star(Game* game);
    void OnUpdate(float deltaTime) override;
    void ManageState();

private:
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
};