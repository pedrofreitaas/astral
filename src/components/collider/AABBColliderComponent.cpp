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

void AABBColliderComponent::CallHorizontalCollisionCallbacks(const float overlap, class AABBColliderComponent *other, IgnoreOption thisColliderIgnoreOption, IgnoreOption otherColliderIgnoreOption)
{
    if (thisColliderIgnoreOption != IgnoreOption::IgnoreCallback)
        mOwner->OnHorizontalCollision(overlap, other);

    if (otherColliderIgnoreOption != IgnoreOption::IgnoreCallback)
        other->GetOwner()->OnHorizontalCollision(-overlap, this);
}

void AABBColliderComponent::CallVerticalCollisionCallbacks(const float overlap, class AABBColliderComponent *other, IgnoreOption thisColliderIgnoreOption, IgnoreOption otherColliderIgnoreOption)
{
    if (thisColliderIgnoreOption != IgnoreOption::IgnoreCallback)
        mOwner->OnVerticalCollision(overlap, other);

    if (otherColliderIgnoreOption != IgnoreOption::IgnoreCallback)
        other->GetOwner()->OnVerticalCollision(-overlap, this);
}

float AABBColliderComponent::DetectHorizontalCollision(RigidBodyComponent *rigidBody)
{
    if (!mIsEnabled)
        return false;

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(mOwner->GetCenter(), 2);

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent *a, AABBColliderComponent *b)
              { return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq()); });

    for (auto &collider : colliders)
    {
        if (collider == this || !collider->IsEnabled())
            continue;

        if (collider->GetLayer() == mLayer)
            continue;

        IgnoreOption otherColliderIgnoreOption = collider->CheckLayerIgnored(mLayer);

        if (otherColliderIgnoreOption == IgnoreOption::Both)
            continue;

        IgnoreOption thisColliderIgnoreOption = CheckLayerIgnored(collider->GetLayer());

        if (thisColliderIgnoreOption == IgnoreOption::Both)
            continue;

        if (!Intersect(*collider))
            continue;

        float overlap = GetMinHorizontalOverlap(collider);

        bool bothTangible = collider->IsTangible() && mIsTangible;

        if (
            bothTangible &&
            thisColliderIgnoreOption != IgnoreOption::IgnoreResolution)
        {
            // it's important this happens before calling the callbacks
            ResolveHorizontalCollisions(rigidBody, overlap);
            CallHorizontalCollisionCallbacks(overlap, collider, thisColliderIgnoreOption, otherColliderIgnoreOption);
            return overlap;
        }

        CallHorizontalCollisionCallbacks(overlap, collider, thisColliderIgnoreOption, otherColliderIgnoreOption);
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

        if (collider->GetLayer() == mLayer)
            continue;

        IgnoreOption otherColliderIgnoreOption = collider->CheckLayerIgnored(mLayer);

        if (otherColliderIgnoreOption == IgnoreOption::Both)
            continue;

        IgnoreOption thisColliderIgnoreOption = CheckLayerIgnored(collider->GetLayer());

        if (thisColliderIgnoreOption == IgnoreOption::Both)
            continue;

        if (!Intersect(*collider))
            continue;

        float overlap = GetMinVerticalOverlap(collider);

        bool bothTangible = collider->IsTangible() && mIsTangible;

        if (
            bothTangible &&
            thisColliderIgnoreOption != IgnoreOption::IgnoreResolution)
        {
            // it's important this happens before calling the callbacks
            ResolveVerticalCollisions(rigidBody, overlap);
            CallVerticalCollisionCallbacks(overlap, collider, thisColliderIgnoreOption, otherColliderIgnoreOption);
            return overlap;
        }

        CallVerticalCollisionCallbacks(overlap, collider, thisColliderIgnoreOption, otherColliderIgnoreOption);
    }

    return 0.0f;
}

void AABBColliderComponent::ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minXOverlap)
{
    constexpr float epsilon = 0.001f; // Small separation buffer
    float adjustment = minXOverlap + (minXOverlap > 0 ? epsilon : -epsilon);
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(adjustment, 0.0f));
    rigidBody->ResetVelocityX();
}

void AABBColliderComponent::ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minYOverlap)
{
    constexpr float epsilon = 0.001f; // Small separation buffer
    float adjustment = minYOverlap + (minYOverlap > 0 ? epsilon : -epsilon);
    mOwner->SetPosition(mOwner->GetPosition() - Vector2(0.0f, adjustment));
    rigidBody->ResetVelocityY();
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

void AABBColliderComponent::MaintainInCamera()
{
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();
    Vector2 getUpperLeftBorder = GetMin();
    Vector2 getBottomRightBorder = GetMax();
    Vector2 offset = GetOffset();
    int maxXBoundary = cameraPos.x + mOwner->GetGame()->GetWindowWidth();
    int maxYBoundary = cameraPos.y + mOwner->GetGame()->GetWindowHeight();

    if (getUpperLeftBorder.x < 0)
    {
        ResolveHorizontalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getUpperLeftBorder.x);
    }
    else if (getBottomRightBorder.x > maxXBoundary)
    {
        ResolveHorizontalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getBottomRightBorder.x - maxXBoundary);
    }

    if (getUpperLeftBorder.y < 0)
    {
        ResolveVerticalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getUpperLeftBorder.y);
    }
    else if (getBottomRightBorder.y > maxYBoundary)
    {
        ResolveVerticalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getBottomRightBorder.y - maxYBoundary);
    }
}

void AABBColliderComponent::MaintainInMap()
{
    int maxXBoundary = mOwner->GetGame()->GetMapWidth();
    int maxYBoundary = mOwner->GetGame()->GetMapHeight();
    Vector2 getUpperLeftBorder = GetMin();
    Vector2 getBottomRightBorder = GetMax();

    if (getUpperLeftBorder.x < 0)
    {
        ResolveHorizontalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getUpperLeftBorder.x);
    }
    else if (getBottomRightBorder.x > maxXBoundary)
    {
        ResolveHorizontalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getBottomRightBorder.x - maxXBoundary);
    }

    if (getUpperLeftBorder.y < 0)
    {
        ResolveVerticalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getUpperLeftBorder.y);
    }
    else if (getBottomRightBorder.y > maxYBoundary)
    {
        ResolveVerticalCollisions(
            mOwner->GetComponent<RigidBodyComponent>(),
            getBottomRightBorder.y - maxYBoundary);
    }
}

bool AABBColliderComponent::IsSegmentIntersecting(const Vector2 &a, const Vector2 &b)
{
    // choose number of sample points proportional to segment length
    float length = (b - a).Length();
    const int minPoints = 4;
    const int maxPoints = 500;
    const float density = 0.5f;
    int totalPoints = (int)std::ceil(length * density);
    totalPoints = std::max(minPoints, std::min(maxPoints, totalPoints));

    // get points along the segment
    for (int i = 0; i <= totalPoints; i++)
    {
        float t = (float)i / (float)totalPoints;
        Vector2 point = Vector2::Lerp(a, b, t);
        if (point.x >= GetMin().x && point.x <= GetMax().x &&
            point.y >= GetMin().y && point.y <= GetMax().y)
        {
            return true;
        }
    }
    return false;
}

bool AABBColliderComponent::IsCollidingRect(const SDL_Rect &rect) const
{
    Vector2 min = GetMin();
    Vector2 max = GetMax();

    return (min.x < rect.x + rect.w &&
            max.x > rect.x &&
            min.y < rect.y + rect.h &&
            max.y > rect.y);
}

void AABBColliderComponent::IgnoreLayer(ColliderLayer layer, IgnoreOption option)
{
    mIgnoredLayersOptions[layer] = option;
}

void AABBColliderComponent::IgnoreLayers(const std::vector<ColliderLayer> &layers, IgnoreOption option)
{
    for (const auto &layer : layers)
    {
        IgnoreLayer(layer, option);
    }
}

void AABBColliderComponent::SetIgnoreLayers(const std::vector<ColliderLayer> &layers, IgnoreOption option)
{
    mIgnoredLayersOptions.clear();
    for (const auto &layer : layers)
    {
        IgnoreLayer(layer, option);
    }
}

IgnoreOption AABBColliderComponent::CheckLayerIgnored(ColliderLayer layer) const
{
    if (mIgnoredLayersOptions.find(layer) != mIgnoredLayersOptions.end())
    {
        return mIgnoredLayersOptions.at(layer);
    }

    return IgnoreOption::None;
}

void AABBColliderComponent::SetBB(const SDL_Rect *rect)
{
    mOffset = Vector2((float)rect->x, (float)rect->y);
    mWidth = rect->w;
    mHeight = rect->h;
}

// Checks if there is a tile wall within the specified horizontal distance
// -1 for left, 0 for none, 1 for right
// distance must be bigger than episilon used in collision resolution
int AABBColliderComponent::IsCloseToTileWallHorizontally(float distance)
{
    if (!mIsEnabled)
        return false;

    Vector2 center = GetCenter();
    Vector2 size = Vector2((float)mWidth, (float)mHeight);

    // Use spatial hashing to get nearby colliders
    auto colliders = mOwner->GetGame()->GetNearbyColliders(center, 2);

    std::sort(colliders.begin(), colliders.end(), [this](AABBColliderComponent *a, AABBColliderComponent *b)
              { return Math::Abs((a->GetCenter() - GetCenter()).LengthSq() < (b->GetCenter() - GetCenter()).LengthSq()); });

    for (auto &collider : colliders)
    {
        if (collider == this || !collider->IsEnabled())
            continue;

        if (collider->GetLayer() != ColliderLayer::Blocks)
            continue;

        if (collider->IsSegmentIntersecting(
                Vector2(center.x - size.x - distance, center.y),
                Vector2(center.x, center.y)))
        {
            return -1;
        }

        if (collider->IsSegmentIntersecting(
                Vector2(center.x, center.y),
                Vector2(center.x + size.x + distance, center.y)))
        {
            return 1;
        }
    }

    return 0;
}