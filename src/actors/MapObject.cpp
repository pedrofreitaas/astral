#include <stdexcept>
#include "./MapObject.h"
#include "../core/Game.h"
#include "../actors/enemies/Sith.h"
#include "../actors/enemies/Zod.h"
#include "../actors/enemies/Quasar.h"
#include "../actors/Zoe.h"
#include "../actors/traps/Shuriken.h"
#include "../actors/traps/Spear.h"
#include "../actors/traps/Spikes.h"
#include "../actors/Portal.h"

MapObject::MapObject(Game *game, int inId, const std::string &ev, const std::string &func_name,
                     const Vector2 &pos, const Vector2 &size, const json &parameters)
    : Actor(game, 1, true), mID(inId), mEvent(ev), mFunctionName(func_name),
      mIsPlayerInside(false), mWasPlayerInside(false),
      mParameters(parameters), mCloseToCenterDistanceSQ(0.f),
      mIsPlayerContainedInMe(false)
{
    if (ev != "in" && ev != "out" && ev != "enter" && ev != "exit" && ev != "closeToCenter" && ev != "contains" // always player related
        && ev != "atStart"                                                                  // not related to player
    )
    {
        throw std::runtime_error("MapObject event must be 'in', 'out', 'enter', 'exit', 'closeToCenter' or 'atStart'");
    }

    if (ev == "closeToCenter" && !parameters.contains("distance"))
    {
        throw std::runtime_error("MapObject event 'closeToCenter' requires 'distance' parameter");
    }

    if (ev == "closeToCenter")
    {
        float dist = parameters["distance"].get<float>();
        mCloseToCenterDistanceSQ = dist * dist; // store squared distance
    }

    if (mFunctionName == "play_cutscene" && !parameters.contains("cutscene_name"))
    {
        throw std::runtime_error("MapObject function 'play_cutscene' requires 'cutscene_name' parameter");
    }

    if (mFunctionName == "spawn_entity" && !parameters.contains("entity_code"))
    {
        throw std::runtime_error("MapObject function 'spawn_entity' requires 'entity_code' parameter");
    }

    SetPosition(pos);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f);
    mRigidBodyComponent->SetApplyGravity(false);
    mRigidBodyComponent->SetApplyFriction(false);

    mColliderComponent = new AABBColliderComponent(
        this, 
        0, 0, 
        size.x, size.y,
        ColliderLayer::Objects,
        false, 
        10);
    
    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::Blocks
    });

    if (ev == "atStart")
    {
        CallMyFunction();
    }
}

void MapObject::CallMyFunction()
{
    if (mFunctionName == "log")
    {
        Log();
        return;
    }

    if (mFunctionName == "play_cutscene")
    {
        PlayCutscene();
        return;
    }

    if (mFunctionName == "spawn_entity")
    {
        SpawnEntity();
        return;
    }

    if (mFunctionName == "teleport_to_checkpoint")
    {
        TeleportToCheckpoint();
        return;
    }

    throw std::runtime_error("MapObject unknown function name: " + mFunctionName);
}

void MapObject::OnUpdate(float deltaTime)
{
    // do logic before because components update before actor update
    float distanceToPlayerSQ = 9999999;
    
    auto *zoe = mGame->GetZoe();

    if (zoe)
    {
        distanceToPlayerSQ = (zoe->GetCenter() - GetCenter()).LengthSq();
    }

    // if - return isnt welcome here, because there is a final operation.

    if (mEvent == "in" && mIsPlayerInside)
    {
        CallMyFunction();
    }
    
    else if (mEvent == "out" && !mIsPlayerInside)
    {
        CallMyFunction();
    }
        
    else if (mEvent == "enter" && mIsPlayerInside && !mWasPlayerInside)
    {
        CallMyFunction();
    }
        
    else if (mEvent == "exit" && !mIsPlayerInside && mWasPlayerInside)
    {
        CallMyFunction();
    }
    
    else if (mEvent == "closeToCenter" && distanceToPlayerSQ <= mCloseToCenterDistanceSQ)
    {
        CallMyFunction();
    }

    else if (mEvent == "contains" && mIsPlayerContainedInMe)
    {
        CallMyFunction();
    }

    mWasPlayerInside = mIsPlayerInside;
    mIsPlayerInside = false;
    mIsPlayerContainedInMe = false;
}

void MapObject::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mIsPlayerInside = true;
        mIsPlayerContainedInMe = other->IsContainedIn(*mColliderComponent);
    }
}

void MapObject::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Player)
    {
        mIsPlayerInside = true;
        mIsPlayerContainedInMe = other->IsContainedIn(*mColliderComponent);
    }
}

void MapObject::Log()
{
    SDL_Log("MapObject function 'log' called");
}

void MapObject::PlayCutscene()
{
    // if (mParameters.contains("cutscene_name")) verified in constructor
    std::string cutsceneName = mParameters["cutscene_name"].get<std::string>();
    
    mGame->StartCutscene(cutsceneName);
    SetState(ActorState::Destroy);
}

void MapObject::SpawnEntity()
{
    // if (!mParameters.contains("entity_code")) verified in constructor
    EntityCode code = static_cast<EntityCode>(mParameters["entity_code"].get<int>());
    
    switch (code)
    {
    case EntityCode::Zoe:
        new Zoe(mGame, 2000.0f, GetCenter());
        break;
    case EntityCode::Sith:
        new Sith(mGame, GetCenter());
        break;
    case EntityCode::Zod:
        new Zod(mGame, 1200.f, GetCenter());
        break;
    case EntityCode::Shuriken:
        new Shuriken(mGame, GetCenter());
        break;
    case EntityCode::Spear:
        new Spear(mGame, GetCenter());
        break;
    case EntityCode::Spikes:
        new Spikes(mGame, GetCenter());
        break;
    case EntityCode::Portal:
        new Portal(mGame, GetCenter());
        break;
    case EntityCode::Quasar:
        new Quasar(mGame, GetCenter());
        break;
    default:
        throw std::runtime_error("MapObject unknown SpawnCode");
        break;
    }

    SetState(ActorState::Destroy);
}


void MapObject::TeleportToCheckpoint()
{
    GetGame()->GetZoe()->TeleportToCheckpoint();
    // SetState(ActorState::Destroy);
}