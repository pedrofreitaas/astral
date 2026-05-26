#include "AIMovementComponent.h"
#include "../../actors/Actor.h"
#include "../../actors/Zoe.h"
#include "../../core/Game.h"
#include "../../core/SpatialHashing.h"
#include "../../actors/Enemy.h"

const float MAX_CRAZINESS = 1.f;
const float MIN_CRAZINESS = 0.f;

AIMovementComponent::AIMovementComponent(
    class Actor *owner, float fowardSpeed,
    TypeOfMovement typeOfMovement, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering),
      mPreviousMovementState(MovementState::Wandering), mInteligence(0.0f),
      mCraziness(craziness), mSpeed(fowardSpeed), mTypeOfMovement(typeOfMovement),
      mOwnerEnemy(nullptr), mCrazyDecisionTimer(nullptr),
      mBlockChangeForceTimer(nullptr), mSpeedFlipped(false), mObstaclesAroundCenters()
{
    if (mOwner->GetComponent<RigidBodyComponent>() == nullptr)
    {
        throw std::runtime_error("AIMovementComponent::AIMovementComponent: Owner missing RigidBodyComponent");
    }

    if (mCraziness > MAX_CRAZINESS)
        mCraziness = MAX_CRAZINESS;
    if (mCraziness < MIN_CRAZINESS)
        mCraziness = MIN_CRAZINESS;

    mOwnerEnemy = dynamic_cast<class Enemy *>(owner);

    TimerComponent* mOwnerTimerComponent = mOwner->GetComponent<TimerComponent>();
    if (mOwnerTimerComponent) {
        mCrazyDecisionTimer = mOwnerTimerComponent->AddNotRemovableTimer(3.f, nullptr);
    } else {
        throw std::runtime_error("AIMovementComponent::AIMovementComponent: Owner missing TimerComponent");
    }
}

void AIMovementComponent::PopulateObstaclesAround()
{
    auto rigidBody = mOwner->GetComponent<RigidBodyComponent>();

    if (!rigidBody) return;

    std::vector<ColliderLayer> obstacleColliders = {
        ColliderLayer::Spikes, ColliderLayer::SpikesBlock,
        ColliderLayer::SpearBlock, ColliderLayer::SpearTip,
        ColliderLayer::Shuriken
    };

    std::vector<AABBColliderComponent *> closeColliders = mOwner->GetGame()->GetNearbyColliders(
        mOwner->GetCenter(),
        3);

    mObstaclesAroundCenters.clear();

    SDL_Rect threatRect = mOwnerEnemy->GetThreatRect();

    for (const auto &collider : closeColliders)
    {
        if (
            std::find(
                obstacleColliders.begin(),
                obstacleColliders.end(),
                collider->GetLayer()) == obstacleColliders.end())
        {
            continue;
        }

        if (collider->IsCollidingRect(threatRect))
        {
            mObstaclesAroundCenters.push_back(collider->GetCenter());
        }
    }
}

void AIMovementComponent::Sense(float deltaTime)
{
    PopulateObstaclesAround();
}

void AIMovementComponent::Plan(float deltaTime)
{
    mInteligence += deltaTime * .1f;
    if (mInteligence > 1.f) mInteligence = 0.f;

    switch (mMovementState)
    {
    case MovementState::Seeking:
    {   
        break;
    }

    case MovementState::Wandering:
    {
        if (CrazyDecision()) {
            SetMovementState(MovementState::Patrolling);
            break;
        }

        break;
    }

    case MovementState::Patrolling:
    {
        if (CrazyDecision()) {
            SetMovementState(MovementState::Wandering);
            break;
        }

        break;
    }

    default:
        break;
    }
}

void AIMovementComponent::Act(float deltaTime)
{
    RigidBodyComponent *rb = mOwner->GetComponent<RigidBodyComponent>();
    AABBColliderComponent *collider = mOwner->GetComponent<AABBColliderComponent>();

    if (!collider || !rb)
    {
        throw std::runtime_error("AIMovementComponent::Act: Owner missing RigidBodyComponent or AABBColliderComponent");
    }

    // not on ground cant move.
    if (!rb->GetOnGround() && rb->GetApplyGravity()) return;

    Vector2 dir;

    switch (GetMovementState())
    {
    case MovementState::Wandering:
    {
        dir = Vector2(mInteligence >= .5f ? 1 : -1, 0.f);
        
        if (mSpeedFlipped) {
            dir *= -1.f;
        }

        break;
    }

    case MovementState::Seeking:
    {
        if (CanBePressingPlayerAgainstWall()) {
            dir = Vector2(0.f, 0.f);
            break;
        }

        Vector2 toPlayer = mOwnerEnemy->GetLastSeenPlayerCenter() - mOwner->GetCenter();

        if (mOwnerEnemy->GetLastSeenPlayerDistanceSquared() < 400.f) break;

        toPlayer.Normalize();

        dir = toPlayer;

        // has a clear goal, doesnt need to flip
        // if (mSpeedFlipped) {
        //     force *= -1.f;
        // }
        
        break;
    }

    case MovementState::Patrolling:
    {
        Vector2 toSpawn = mOwnerEnemy->GetSpawnPosition() - mOwner->GetPosition();
        
        if (toSpawn.LengthSq() < 100.f) {
            dir = Vector2::Zero;
            break;
        }

        toSpawn.Normalize();
        
        dir = toSpawn;

        if (mSpeedFlipped) {
            dir *= -1.f;
        }

        break;
    }

    default:
        SetMovementState(MovementState::Wandering);
        break;
    }

    if (IsDangerousToMoveAround())
    {
        Vector2 avoidDir = Vector2(0.f, 0.f);
        float norm = 0.f;
        float modifier = 0.f;
        float limit = mSpeed * .5f;

        for (const auto& obstacleCenter : mObstaclesAroundCenters) {
            Vector2 awayFromObstacle = mOwner->GetCenter() - obstacleCenter;
            norm = awayFromObstacle.Length() + 0.001f;
            awayFromObstacle *= 1.f/norm;
            
            modifier = (limit-norm);
            if (modifier < 0) modifier = 0.f;
            
            avoidDir += awayFromObstacle * modifier;
        }

        dir += avoidDir;
        dir.Normalize();
    }

    if (mTypeOfMovement == TypeOfMovement::Walker) {
        dir.y = 0.f;
        rb->ApplyForce(dir * mSpeed);
    }
    else {
        rb->SetVelocity(dir * mSpeed);
    }
}

bool AIMovementComponent::CrazyDecision()
{
    TimerComponent* mOwnerTimerComponent = mOwner->GetComponent<TimerComponent>();

    if (!mOwnerTimerComponent) {
        return false;
    }

    if (mOwnerTimerComponent->checkTimerRemaining(mCrazyDecisionTimer) > 0.f) {
        return false;
    }

    float randomValue = Math::RandRange(MIN_CRAZINESS, MAX_CRAZINESS);
    bool crazy = (randomValue <= mCraziness);

    if (crazy) mCrazyDecisionTimer->Restart();

    return crazy;
}

bool AIMovementComponent::CrazyDecision(float modifier)
{
    TimerComponent* mOwnerTimerComponent = mOwner->GetComponent<TimerComponent>();

    if (!mOwnerTimerComponent) {
        return false;
    }

    if (mOwnerTimerComponent->checkTimerRemaining(mCrazyDecisionTimer) > 0.f) {
        return false;
    }

    mCrazyDecisionTimer->Restart();
    float randomValue = Math::RandRange(MIN_CRAZINESS, MAX_CRAZINESS);
    return (randomValue <= (mCraziness * modifier));
}

void AIMovementComponent::Update(float deltaTime)
{
    if (mOwner->GetBehaviorState() != BehaviorState::Moving)
        return;

    Sense(deltaTime);
    Plan(deltaTime);
    Act(deltaTime);
}

void AIMovementComponent::SeekPlayer()
{
    if (GetMovementState() == MovementState::Seeking)
        return;

    SetMovementState(MovementState::Seeking);
}

void AIMovementComponent::LoosePlayer()
{
    if (GetMovementState() != MovementState::Seeking)
        return;

    SetMovementState(MovementState::Patrolling);
}

void AIMovementComponent::LogState()
{
    switch (GetMovementState())
    {
    case MovementState::Wandering:
        SDL_Log("AIMovementComponent::LogState: Wandering");
        break;
    case MovementState::Seeking:
        SDL_Log("AIMovementComponent::LogState: Seeking");
        break;
    case MovementState::Patrolling:
        SDL_Log("AIMovementComponent::LogState: Patrolling");
        break;
    default:
        SDL_Log("AIMovementComponent::LogState: Unknown State");
        break;
    }
}

void AIMovementComponent::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (
        other->GetLayer() == ColliderLayer::Blocks ||
        other->GetLayer() == ColliderLayer::EnemyBlocker
    )
    {
        if (mMovementState != MovementState::Seeking) {
            mSpeedFlipped = true;
            mOwner->GetComponent<TimerComponent>()->AddTimer(1.f, [this]() { mSpeedFlipped = false; });    
        }

        else if (mTypeOfMovement == TypeOfMovement::Flier) {
            Vector2 toPlayer = mOwnerEnemy->GetLastSeenPlayerCenter() - mOwner->GetCenter();

            if (Math::Abs(toPlayer.y) > 20.f) {
                auto rb = mOwner->GetComponent<RigidBodyComponent>();
                rb->SetVelocity(Vector2(0.f, Math::Sign(toPlayer.y) * mSpeed));
            }
        }
    }
}

void AIMovementComponent::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (
        other->GetLayer() == ColliderLayer::Blocks ||
        other->GetLayer() == ColliderLayer::EnemyBlocker
    )
    {
        if (mTypeOfMovement == TypeOfMovement::Flier && mMovementState == MovementState::Seeking) {
            Vector2 toPlayer = mOwnerEnemy->GetLastSeenPlayerCenter() - mOwner->GetCenter();

            if (Math::Abs(toPlayer.x) > 20.f) {
                auto rb = mOwner->GetComponent<RigidBodyComponent>();
                rb->SetVelocity(Vector2(Math::Sign(toPlayer.x) * mSpeed, 0.f));
            }
        }
    }
}

void AIMovementComponent::BoostToPlayer(float intensity)
{
    RigidBodyComponent *rb = mOwner->GetComponent<RigidBodyComponent>();
    if (!rb)
        return;

    Vector2 boostDirection = mOwnerEnemy->GetLastSeenPlayerCenter() - mOwner->GetCenter();
    boostDirection.Normalize();

    Vector2 boostForce = boostDirection * intensity;
    rb->ApplyImpulse(boostForce);
}

void AIMovementComponent::SetMovementState(MovementState state)
{
    mPreviousMovementState = mMovementState;
    mMovementState = state;

    LogState();
}

bool AIMovementComponent::CanBePressingPlayerAgainstWall() const
{
    if (mMovementState != MovementState::Seeking) return false;

    Vector2 toPlayer = mOwnerEnemy->GetLastSeenPlayerCenter() - mOwner->GetCenter();
    float distanceToPlayerSQ = mOwnerEnemy->GetDistanceToPlayerSquared(); // cant be get distance to last seen!

    if (distanceToPlayerSQ > 900.f) return false;

    AABBColliderComponent* playerColliderComponent = mOwnerEnemy->
        GetGame()->
        GetZoe()->
        GetComponent<AABBColliderComponent>();
    
    if (!playerColliderComponent) return false;

    int playerCloseToWall = playerColliderComponent->IsCloseToTileWallHorizontally(1.f);

    return playerCloseToWall != 0;
}