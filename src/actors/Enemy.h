#pragma once

#include "Actor.h"
#include "ZoeFireball.h"
#include <SDL.h>

class Enemy : public Actor
{
public:
    explicit Enemy(Game *game, const Vector2 &position, float maxSeeDistance=300.f, float minSeeDistance=40.f);
    ~Enemy() override;

    void OnUpdate(float deltaTime) override;
    virtual void ManageState() = 0;

    Vector2 GetCurrentAppliedForce(float modifier=0.f) const;
    Vector2 GetCurrentVelocity(float modifier=0.f) const;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) override;

    SDL_Rect GetThreatRect() const;
    std::vector<Vector2> GetObstaclesAroundCenters() const;

    float GetMaxSeeDistance() const { return mMaxSeeDistance; }
    float GetMinSeeDistance() const { return mMinSeeDistance; }
    float GetFovAngle() const;

    bool isAISeeking() const;

protected:
    friend class AIMovementComponent;

    class RigidBodyComponent *mRigidBodyComponent;
    class DrawAnimatedComponent *mDrawComponent;
    class AABBColliderComponent *mColliderComponent;
    class AIMovementComponent *mAIMovementComponent;

    virtual void ManageAnimations() = 0;
    virtual void AnimationEndCallback(std::string animationName) = 0;

    bool PlayerOnSight(float angle=0.f);
    bool PlayerOnFov();

    void Freeze() override;
    void StopFreeze() override;

    bool GetHasSeenPlayerThisFrame() const { return mHasSeenPlayerThisFrame; }
    Vector2 GetLastSeenPlayerCenter() const { return mLastSeenPlayerCenter; }
    Vector2 GetSpawnPosition() const { return mSpawnPosition; }

    bool HasSeenPlayerThisFrame() const { return mHasSeenPlayerThisFrame; }
    bool IsPlayerOnSightThisFrame() const { return mPlayerOnSightThisFrame; }
    float GetHowLongLastSeenPlayer() const { return mHowLongLastSeenPlayer; }
    float GetDistanceToPlayerSquared() const { return mDistanceToPlayerSquared; }
    float GetLastSeenPlayerDistanceSquared() const { return mLastSeenPlayerDistanceSquared; }

    bool HasSeenPlayerRecently(float timeThreshold=2.f) const { return mHowLongLastSeenPlayer <= timeThreshold; }

private:
    float mMaxSeeDistance, mMinSeeDistance;
    bool mHasSeenPlayerThisFrame, mPlayerOnSightThisFrame;
    Vector2 mLastSeenPlayerCenter, mSpawnPosition;
    float mHowLongLastSeenPlayer, mDistanceToPlayerSquared, mLastSeenPlayerDistanceSquared;
};