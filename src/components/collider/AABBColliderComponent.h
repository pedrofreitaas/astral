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
    Objects,
    Item
};

class AABBColliderComponent : public Component
{
public:
    AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h,
                                ColliderLayer layer,  bool isTangible = true, int updateOrder = 10);
    ~AABBColliderComponent() override;

    bool Intersect(const AABBColliderComponent& b) const;

    float DetectHorizontalCollision(RigidBodyComponent *rigidBody);
    float DetectVerticalCollision(RigidBodyComponent *rigidBody);

    bool IsOnCamera();

    Vector2 GetMin() const;
    Vector2 GetMax() const;
    Vector2 GetCenter() const;
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    Vector2 GetOffset() const { return mOffset; }
    void SetOffset(const Vector2& offset) { mOffset = offset; }
    ColliderLayer GetLayer() const { return mLayer; }
    bool IsTangible() const { return mIsTangible; }

    void MaintainInbound();

private:
    float GetMinVerticalOverlap(AABBColliderComponent* b) const;
    float GetMinHorizontalOverlap(AABBColliderComponent* b) const;

    void ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);
    void ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);

    Vector2 mOffset;
    int mWidth;
    int mHeight;
    bool mIsTangible;

    ColliderLayer mLayer;
};