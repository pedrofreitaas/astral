//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AABBColliderComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"
#include <algorithm>

AABBColliderComponent::AABBColliderComponent(class Actor *owner, int dx, int dy, int w, int h,
                                             ColliderLayer layer, bool isTangible, int updateOrder)
    : Component(owner, updateOrder), mOffset(Vector2((float)dx, (float)dy)), 
    mWidth(w), mHeight(h), mLayer(layer), mIsTangible(isTangible)
{
}

AABBColliderComponent::~AABBColliderComponent()
{
}

Vector2 AABBColliderComponent::GetMin() const
{
    return mOwner->GetPosition() + mOffset;
}

Vector2 AABBColliderComponent::GetMax() const
{
    return GetMin() + Vector2((float)mWidth, (float)mHeight);
}

Vector2 AABBColliderComponent::GetCenter() const
{
    return GetMin() + Vector2((float)mWidth / 2.0f, (float)mHeight / 2.0f);
}

bool AABBColliderComponent::Intersect(const AABBColliderComponent &b) const
{
    return (GetMin().x < b.GetMax().x && GetMax().x > b.GetMin().x &&
            GetMin().y < b.GetMax().y && GetMax().y > b.GetMin().y);
}

float AABBColliderComponent::GetMinVerticalOverlap(AABBColliderComponent *b) const
{
    float top = GetMin().y - b->GetMax().y;
    float down = GetMax().y - b->GetMin().y;

    return (Math::Abs(top) < Math::Abs(down)) ? top : down;
}

float AABBColliderComponent::GetMinHorizontalOverlap(AABBColliderComponent *b) const
{
    float right = GetMax().x - b->GetMin().x; // Right
    float left = GetMin().x - b->GetMax().x;  // Left

    return (Math::Abs(left) < Math::Abs(right)) ? left : right;
}

float AABBColliderComponent::DetectHorizontalCollision(RigidBodyComponent *rigidBody)
{
    if (!mIsEnabled)
        return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent *a, AABBColliderComponent *b)
              { return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq()); });

    for (auto &collider : colliders)
    {
        if (collider == this || !collider->IsEnabled())
            continue;

        if (Intersect(*collider))
        {
            float overlap = GetMinHorizontalOverlap(collider);

            if (collider->IsTangible() && mIsTangible) {
                ResolveHorizontalCollisions(rigidBody, overlap);
                mOwner->OnHorizontalCollision(overlap, collider);
                return overlap;
            }
            
            mOwner->OnHorizontalCollision(overlap, collider);
        }
    }

    return 0.0f;
}

float AABBColliderComponent::DetectVerticalCollision(RigidBodyComponent *rigidBody)
{
    if (!mIsEnabled)
        return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetPosition());

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent *a, AABBColliderComponent *b)
              { return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq()); });

    for (auto &collider : colliders)
    {
        if (collider == this || !collider->IsEnabled())
            continue;

        if (Intersect(*collider))
        {
            float overlap = GetMinVerticalOverlap(collider);
            
            if (collider->IsTangible() && mIsTangible) {
                ResolveVerticalCollisions(rigidBody, overlap);
                mOwner->OnVerticalCollision(overlap, collider);
                return overlap;
            }

            mOwner->OnVerticalCollision(overlap, collider);
        }
    }
    
    return 0.0f;
}

void AABBColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minXOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(minXOverlap, 0.0f));
    rigidBody->SetVelocity(Vector2(0.f, rigidBody->GetVelocity().y));
}

void AABBColliderComponent::ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minYOverlap)
{
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(0.0f, minYOverlap));
    rigidBody->SetVelocity(Vector2(rigidBody->GetVelocity().x, 0.f));
}

bool AABBColliderComponent::IsOnCamera()
{
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();
    Vector2 min = GetMin();
    Vector2 max = GetMax();

    return (min.x < cameraPos.x + mOwner->GetGame()->GetWindowWidth() &&
            max.x > cameraPos.x &&
            min.y < cameraPos.y + mOwner->GetGame()->GetWindowHeight() &&
            max.y > cameraPos.y);
}

void AABBColliderComponent::MaintainInbound()
{
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();
    Vector2 getUpperLeftBorder = GetMin();
    Vector2 getBottomRightBorder = GetMax();
    Vector2 offset = GetOffset();
    int maxXBoundary = cameraPos.x + mOwner->GetGame()->GetWindowWidth();
    int maxYBoundary = cameraPos.y + mOwner->GetGame()->GetWindowHeight();

    if (getUpperLeftBorder.x < 0)
    {
        mOwner->SetPosition(Vector2(-offset.x, mOwner->GetPosition().y));
    }
    else if (getBottomRightBorder.x > maxXBoundary)
    {
        mOwner->SetPosition(Vector2(maxXBoundary - mWidth - offset.x, mOwner->GetPosition().y));
    }

    if (getUpperLeftBorder.y < 0)
    {
        mOwner->SetPosition(Vector2(mOwner->GetPosition().x, -offset.y));
    }
    else if (getBottomRightBorder.y > maxYBoundary)
    {
        mOwner->SetPosition(Vector2(mOwner->GetPosition().x, maxYBoundary - mHeight - offset.y));
    }
}