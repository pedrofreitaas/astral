#include "./Star.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"

Star::Star(Game *game) : 
    Actor(game)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f);
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->SetApplyFriction(false);
    
    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Star/texture.png",
        "../assets/Sprites/Star/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("twinkle", {1});
    mDrawComponent->SetAnimation("twinkle");
    mDrawComponent->SetAnimFPS(1.0f);
}

void Star::OnUpdate(float deltaTime)
{
    // Stars don't do anything for now
}