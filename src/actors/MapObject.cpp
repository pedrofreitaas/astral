#include <stdexcept>
#include "./MapObject.h"
#include "../core/Game.h"

void OBJECT_LOG() {
    SDL_Log("MapObject function 'log' called");
}

MapObject::MapObject(Game *game, int inId, const std::string &ev, const std::string &func_name, const Vector2 &pos, const Vector2 &size)
    : Actor(game), mID(inId), mEvent(ev), mFunctioNName(func_name), 
    mIsPlayerInside(false), mWasPlayerInside(false)
{
    if (ev != "in" && ev != "out" && ev != "enter" && ev != "exit") {
        throw std::runtime_error("MapObject event must be 'in', 'out', 'enter' or 'exit'");
    }

    if (func_name == "log") {
        mFunction = OBJECT_LOG;
    }

    else {
        throw std::runtime_error("MapObject function '" + func_name + "' not found");
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
    if (mEvent == "in" && mIsPlayerInside) {
        mFunction();
    }
    else if (mEvent == "out" && !mIsPlayerInside) {
        mFunction();
    }
    else if (mEvent == "enter" && mIsPlayerInside && !mWasPlayerInside) {
        mFunction();
    }
    else if (mEvent == "exit" && !mIsPlayerInside && mWasPlayerInside) {
        mFunction();
    }
    
    mWasPlayerInside = mIsPlayerInside;
    mIsPlayerInside = false;
}

void MapObject::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        mIsPlayerInside = true;
    }
}

void MapObject::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{    
    if (other->GetLayer() == ColliderLayer::Player) {
        mIsPlayerInside = true;
    }
}