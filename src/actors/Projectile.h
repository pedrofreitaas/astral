#pragma once
#include "Actor.h"

class Projectile : public Actor
{
public:
    Projectile(
        class Game* game, const std::string &spriteSheetPath, 
        const std::string &spriteSheetData, Vector2 direction, 
        Vector2 position, float speed
    ): Actor(game), mDirection(direction), mSpeed(speed) {};

protected:
    void OnUpdate(float deltaTime) override {
        Vector2 movement = mDirection * mSpeed * deltaTime;
        mRigidBodyComponent->ApplyForce(movement);
    }

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override {
        Kill();
    }

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override {
        Kill();
    }

    virtual void ManageState() = 0;
    virtual void ManageAnimations() = 0;
    virtual void Kill() = 0;

    class DrawAnimatedComponent* mDrawAnimatedComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    
    Vector2 mDirection;
    float mSpeed;
};
