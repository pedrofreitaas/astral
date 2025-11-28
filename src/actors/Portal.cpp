#include "Portal.h"
#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/ai/AIMovementComponent.h"

Portal::Portal(Game *game, const Vector2 &center)
    : Actor(game)
{
    mColliderComponent = new AABBColliderComponent(
        this,
        0, 0,
        26, 48,
        ColliderLayer::Portal,
        false);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Portal/texture.png",
        "../assets/Sprites/Portal/texture.json",
        [this](std::string animationName) {},
        static_cast<int>(DrawLayerPosition::Player) - 1);

    mDrawComponent->AddAnimation("idle", 0, 7);
    mDrawComponent->SetAnimation("idle");

    SetPosition(center - GetHalfSize());
}
