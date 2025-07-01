#include "Projectile.h"
#include "Punk.h"
#include "../Game.h"
#include "../Components/ColliderComponents/CircleColliderComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

Projectile::Projectile(Game *game, ColliderLayer layer, int type)
    : Actor(game)
{
    std::string bullet_path = (type == 1) ? "../Assets/Sprites/Projectile/bullet1.png" :
                                            "../Assets/Sprites/Projectile/bullet2.png";

    mDrawComponent = new DrawSpriteComponent(this, bullet_path, 8, 6, 999);
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f, 0, false);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, 4, 3, layer, true);
}

void Projectile::OnUpdate(float deltaTime)
{
    if (!mColliderComponent->IsOnCamera())
    {
        SetState(ActorState::Destroy);
    }
}

void Projectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks)
    {
        SetState(ActorState::Destroy);
    }
}

void Projectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks)
    {
        SetState(ActorState::Destroy);
    }
}
