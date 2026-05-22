#pragma once
#include <SDL.h>
#include "../../actors/Actor.h"
#include "../Component.h"
#include "../../libs/Math.h"
#include "../RigidBodyComponent.h"
#include "../collider/AABBColliderComponent.h"
#include "../../libs/Math.h"
#include "../../libs/Random.h"
#include "../TimerComponent.h"

enum class TypeOfMovement
{
    Walker,
    Flier
};

enum class MovementState
{
    Wandering, // moving "randomically"
    Seeking, // moving seeking player
    Patrolling // moving around spawn
};

class AIMovementComponent : public Component
{
public:
    AIMovementComponent(
        class Actor* owner, float fowardSpeed, 
        TypeOfMovement typeOfMovement=TypeOfMovement::Walker, 
        float craziness=.005f);
    
    ~AIMovementComponent() override = default;

    float GetSpeed() const { return mSpeed; }
    MovementState GetMovementState() const { return mMovementState; }
    TypeOfMovement GetMovementType() const { return mTypeOfMovement; }
    
    void SetMovementState(MovementState state);
    
    bool CanBePressingPlayerAgainstWall() const;

    void SeekPlayer();
    void LoosePlayer();
    void LogState();

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other);
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other);

    bool CrazyDecision();
    bool CrazyDecision(float modifier);
    
    // applies a force with the same direction of movement considering intensity.
    void BoostToPlayer(float intensity);
    std::vector<Vector2> GetObstaclesAroundCenters() const { return mObstaclesAroundCenters; }

    bool IsDangerousToMoveAround() const { return mObstaclesAroundCenters.size() > 0; }

private:
    void Sense(float deltaTime);
    void Plan(float deltaTime);
    void Act(float deltaTime);
    void Update(float deltaTime) override;
    void PopulateObstaclesAround();

    TypeOfMovement mTypeOfMovement;
    MovementState mMovementState, mPreviousMovementState;
    float mInteligence, mCraziness, mSpeed;
    bool mSpeedFlipped;
    class Enemy* mOwnerEnemy;
    Timer *mCrazyDecisionTimer, *mBlockChangeForceTimer;
    std::vector<Vector2> mObstaclesAroundCenters;
};
