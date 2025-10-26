#include <stdexcept>
#include "./MapObject.h"
#include "../core/Game.h"

MapObject::MapObject(Game *game, int inId, const std::string &ev, const std::string &func_name,
                     const Vector2 &pos, const Vector2 &size, const json &parameters)
    : Actor(game), mID(inId), mEvent(ev), mFunctionName(func_name),
      mIsPlayerInside(false), mWasPlayerInside(false), mParameters(parameters)
{
    if (ev != "in" && ev != "out" && ev != "enter" && ev != "exit")
    {
        throw std::runtime_error("MapObject event must be 'in', 'out', 'enter' or 'exit'");
    }

    SetPosition(pos);

    mRigidBodyComponent = new RigidBodyComponent(this, 0.0f, 0.0f);
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->SetApplyFriction(false);

    mColliderComponent = new AABBColliderComponent(
        this, 0, 0, size.x, size.y,
        ColliderLayer::Objects,
        false, 10);
}

void MapObject::OnUpdate(float deltaTime)
{
    // do logic before because components update before actor update
    if (
        !(
            mEvent == "in" && mIsPlayerInside ||
            mEvent == "out" && !mIsPlayerInside ||
            mEvent == "enter" && mIsPlayerInside && !mWasPlayerInside ||
            mEvent == "exit" && !mIsPlayerInside && mWasPlayerInside))
    {
        return;
    }
    
    if (mFunctionName == "log")
    {
        Log();
    }
    else if (mFunctionName == "play_cutscene")
    {
        if (mParameters.contains("cutscene_name"))
        {
            std::string cutsceneName = mParameters["cutscene_name"].get<std::string>();
            PlayCutscene(cutsceneName);
        }
        else
        {
            throw std::runtime_error("MapObject 'play_cutscene' function requires 'cutscene_name' parameter");
        }
    }
    else {
        throw std::runtime_error("MapObject unknown function name: " + mFunctionName);
    }

    mWasPlayerInside = mIsPlayerInside;
    mIsPlayerInside = false;
}

void MapObject::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mIsPlayerInside = true;
    }
}

void MapObject::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mIsPlayerInside = true;
    }
}

void MapObject::Log()
{
    SDL_Log("MapObject function 'log' called");
}

void MapObject::PlayCutscene(const std::string &cutsceneName)
{
    mGame->StartCutscene(cutsceneName);
}