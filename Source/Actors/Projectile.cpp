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
    mPreviousPosition = GetPosition();

}

bool Projectile::LinesIntersect(const Vector2& p1, const Vector2& p2,
                                const Vector2& q1, const Vector2& q2)
{
    auto orientation = [](const Vector2& a, const Vector2& b, const Vector2& c) -> int
    {
        float val = (b.y - a.y) * (c.x - b.x) -
                    (b.x - a.x) * (c.y - b.y);
        if (fabs(val) < 1e-6f) return 0; // colinear
        return (val > 0) ? 1 : 2;
    };

    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);

    if (o1 != o2 && o3 != o4)
        return true;

    return false;
}


bool Projectile::LineIntersectsAABB(const Vector2& p1, const Vector2& p2, AABBColliderComponent* box)
{
    Vector2 boxMin = box->GetMin();
    Vector2 boxMax = box->GetMax();

    // Testar contra cada lado do AABB
    Vector2 a = Vector2(boxMin.x, boxMin.y);
    Vector2 b = Vector2(boxMax.x, boxMin.y);
    Vector2 c = Vector2(boxMax.x, boxMax.y);
    Vector2 d = Vector2(boxMin.x, boxMax.y);

    if (LinesIntersect(p1, p2, a, b)) return true;
    if (LinesIntersect(p1, p2, b, c)) return true;
    if (LinesIntersect(p1, p2, c, d)) return true;
    if (LinesIntersect(p1, p2, d, a)) return true;

    return false;
}


void Projectile::OnUpdate(float deltaTime)
{
    Vector2 oldPos = mPreviousPosition;
    Vector2 newPos = GetPosition();
   // SDL_Log("[Projectile] Update - Pos: %.1f, %.1f | Prev: %.1f, %.1f",
    //            newPos.x, newPos.y, mPreviousPosition.x, mPreviousPosition.y);
    auto colliders = mGame->GetNearbyColliders(newPos, 2);

    for (auto collider : colliders)
    {
        if (collider->GetLayer() == ColliderLayer::Blocks ||
            collider->GetLayer() == ColliderLayer::Bricks)
        {
            if (LineIntersectsAABB(oldPos, newPos, collider))
            {
                SetState(ActorState::Destroy);
                //SDL_Log("[Projectile] Destroyed at %.1f, %.1f", newPos.x, newPos.y);

                return;
            }
        }
    }


    mPreviousPosition = newPos;

    mDeathTimer -= deltaTime;
    if (mDeathTimer <= 0.0f) {
        SetState(ActorState::Destroy);
        //SDL_Log("[Projectile] Destroyed at %.1f, %.1f", newPos.x, newPos.y);

    }
}

void Projectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    Vector2 currentPos = GetPosition();

    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks) {
        SetState(ActorState::Destroy);
        //SDL_Log("[Projectile] Destroyed at %.1f, %.1f", currentPos.x, currentPos.y);

    }
}

void Projectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    Vector2 currentPos = GetPosition();

    if (other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Bricks) {
        SetState(ActorState::Destroy);
        //SDL_Log("[Projectile] Destroyed at %.1f, %.1f", currentPos.x, currentPos.y);

    }
}
