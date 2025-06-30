#include "Portal.h"

Portal::Portal(Game* game)
    : Actor(game)
{
    mColliderComponent = new AABBColliderComponent(this, 0, 0, 64, 64, ColliderLayer::Portal, true);

    mDrawComponent = new DrawAnimatedComponent(this,
                                               "../Assets/Sprites/Portal/texture.png",
                                               "../Assets/Sprites/Portal/texture.json");

    mDrawComponent->AddAnimation("idle", {0,1,2,3,4,5});
    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);
}

Portal::~Portal() {
}
