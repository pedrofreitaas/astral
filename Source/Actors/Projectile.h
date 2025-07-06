#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(class Game* game, ColliderLayer layer, int type=1);

    void OnUpdate(float deltaTime) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    bool LinesIntersect(const Vector2& p1, const Vector2& p2,
                                    const Vector2& q1, const Vector2& q2);
    bool LineIntersectsAABB(const Vector2& p1, const Vector2& p2, AABBColliderComponent* box);
    Vector2 mPreviousPosition;

private:    
    class DrawSpriteComponent* mDrawComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};