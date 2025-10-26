#pragma once

#include <string>
#include <SDL.h>
#include <functional>
#include "./Actor.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../libs/Json.h"

using json = nlohmann::json;

class Game;

class MapObject: public Actor {
public:
    MapObject(Game *game, int inId, const std::string &ev, const std::string &func_name, 
              const Vector2 &pos, const Vector2 &size, const json &parameters=json::object());

    void OnUpdate(float deltaTime) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    int mID;
    json mParameters;
    std::string mEvent, mFunctionName;
    AABBColliderComponent *mColliderComponent;
    RigidBodyComponent *mRigidBodyComponent; // only to check collision
    bool mIsPlayerInside, mWasPlayerInside;

    void Log();
    void PlayCutscene(const std::string &cutsceneName);
};