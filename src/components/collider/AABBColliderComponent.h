//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "../Component.h"
#include "../../libs/Math.h"
#include "../RigidBodyComponent.h"
#include <vector>
#include <map>
#include <set>

enum class ColliderLayer
{
    Player,
    Enemy,
    Blocks,
    Pole,
    Bricks,
    EnemyProjectile,
    PlayerProjectile,
    Portal,
    Portal2,
    Item
};

class AABBColliderComponent : public Component
{
public:
    // Collider ignore map
    const std::map<ColliderLayer, const std::set<ColliderLayer>> ColliderIgnoreMap = {
        {ColliderLayer::Player, { ColliderLayer::Portal, ColliderLayer::PlayerProjectile, ColliderLayer::Item }},
        {ColliderLayer::Enemy,  { ColliderLayer::EnemyProjectile }},
        {ColliderLayer::Blocks, { ColliderLayer::Blocks }},
        {ColliderLayer::Pole, {}}
    };

    AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h,
                                ColliderLayer layer, bool isStatic = false, int updateOrder = 10);
    ~AABBColliderComponent() override;

    bool Intersect(const AABBColliderComponent& b) const;

    float DetectHorizontalCollision(RigidBodyComponent *rigidBody);
    float DetectVertialCollision(RigidBodyComponent *rigidBody);

    void SetStatic(bool isStatic) { mIsStatic = isStatic; }

    bool IsOnCamera();

    Vector2 GetMin() const;
    Vector2 GetMax() const;
    Vector2 GetCenter() const;
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    Vector2 GetOffset() const { return mOffset; }
    void SetOffset(const Vector2& offset) { mOffset = offset; }
    ColliderLayer GetLayer() const { return mLayer; }

private:
    float GetMinVerticalOverlap(AABBColliderComponent* b) const;
    float GetMinHorizontalOverlap(AABBColliderComponent* b) const;

    void ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);
    void ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);

    Vector2 mOffset;
    int mWidth;
    int mHeight;
    bool mIsStatic;

    ColliderLayer mLayer;
};