#pragma once

#include <string>
#include <SDL.h>
#include "./Actor.h"
#include "../components/collider/AABBColliderComponent.h"
#include <functional>

class Game;

class MapObject: public Actor {
public:
    MapObject(Game *game, int inId, const std::string &ev, const std::string &func_name, const Vector2 &pos, const Vector2 &size);

    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    int mID;
    std::string mEvent, mFunctioNName;
    AABBColliderComponent *mColliderComponent;
    RigidBodyComponent *mRigidBodyComponent; // only to check collision
    bool mIsPlayerInside, mWasPlayerInside;
    std::function<void()> mFunction;
};