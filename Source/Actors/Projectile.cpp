#include "Projectile.h"
#include "Punk.h"
#include "../Game.h"
#include "../Components/ColliderComponents/CircleColliderComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

Projectile::Projectile(Game* game, const float length, const float deathTimer, ColliderLayer layer)
        :Actor(game)
        ,mLength(length)
        ,mDeathTimer(deathTimer)
{
    Vector2 v1 = Vector2(-mLength / 2.0f, 0.0f);
    Vector2 v2 = Vector2(mLength / 2.0f, 0.0f);

    std::vector<Vector2> verts { v1, v2 };

    //mDrawComponent = new DrawPolygonComponent(this, verts,100);
    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Projectile/bullet.png", 4, 3, 100);
    mRigidBodyComponent = new RigidBodyComponent(this, 0.1f, 0, false);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, 7, 6, layer, true);
}

void Projectile::OnUpdate(float deltaTime)
{
    mDeathTimer -= deltaTime;

    if (mDeathTimer <= 0.0f) {
        SetState(ActorState::Destroy);
    }
}

void Projectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks) {
        SetState(ActorState::Destroy);
    }
}

void Projectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks) {
        SetState(ActorState::Destroy);
    }
}
