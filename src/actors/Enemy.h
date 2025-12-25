#pragma once

#include "Actor.h"
#include "ZoeFireball.h"
#include <SDL.h>

class Enemy : public Actor
{
public:
    float PLAYER_ATTACK_KNOCKBACK_FORCE = 2000.f;
    float FIREBALL_KNOCKBACK_FORCE = 12000.f;

    explicit Enemy(Game *game, const Vector2 &position);
    ~Enemy() override;

    void OnUpdate(float deltaTime) override;
    virtual void ManageState() = 0;
    std::vector<SDL_Rect> GetPath() const;

    Vector2 GetCurrentAppliedForce(float modifier);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent *other) override;

protected:
    friend class AIMovementComponent;

    class RigidBodyComponent *mRigidBodyComponent;
    class DrawAnimatedComponent *mDrawComponent;
    class AABBColliderComponent *mColliderComponent;
    class AIMovementComponent *mAIMovementComponent;

    virtual void ManageAnimations() = 0;
    virtual void AnimationEndCallback(std::string animationName) = 0;

    bool PlayerOnSight(float distance = 100.f, float angle=0.f);
    bool PlayerOnFov(float minDistance = 20.f, float maxDistance=250.f);
};