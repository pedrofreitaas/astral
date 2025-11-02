#include "AIMovementComponent.h"
#include "../../actors/Actor.h"
#include "../../actors/Zoe.h"
#include "../../core/Game.h"
#include "../../core/SpatialHashing.h"

const float MAX_CRAZINESS = 1.f;
const float MIN_CRAZINESS = 0.f;

AIMovementComponent::AIMovementComponent(class Actor* owner, float fowardSpeed, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering), 
      mPreviousMovementState(MovementState::Wandering),
      mJumpForceInBlocks(3), mInteligence(0.0f), mCraziness(craziness), mFowardSpeed(fowardSpeed)
{
    if (mCraziness > MAX_CRAZINESS) mCraziness = MAX_CRAZINESS;
    if (mCraziness < MIN_CRAZINESS) mCraziness = MIN_CRAZINESS;
}

void AIMovementComponent::Sense(float deltaTime)
{
}

void AIMovementComponent::Plan(float deltaTime)
{
    switch (GetMovementState())
    {
    case MovementState::Wandering:
        break; // no planning
    case MovementState::Jumping:
        break; // no planning
    case MovementState::FollowingPath:
        break;
    default:
        break;
    }
}

void AIMovementComponent::Act(float deltaTime)
{
    RigidBodyComponent* rb = GetOwnerRigidBody();

    if (!rb) return;

    switch (GetMovementState())
    {
    case MovementState::Wandering:
        if (!rb->GetOnGround()) break;
        
        if (mInteligence >= .5f) rb->ApplyForce(Vector2(mFowardSpeed, 0.f));
        else rb->ApplyForce(Vector2(-mFowardSpeed, 0.f));

        if (CrazyDecision(.5f)) Jump();

        break;

    case MovementState::Jumping:
        if (rb->GetOnGround())
        {
            SetMovementState(mPreviousMovementState);
        }
        break;
    
    case MovementState::FollowingPath: {
        if (mPath.empty()) {
            SetMovementState(MovementState::Wandering);
            break;
        }

        if (!rb->GetOnGround()) break;

        Vector2 ownerCenter = mOwner->GetCenter();
        Vector2 targetPos = mPath.front();

        Vector2 toTarget = targetPos - ownerCenter;

        float jumpThreshold = Game::TILE_SIZE * 0.75f;
        
        float distanceToTarget = toTarget.Length();

        toTarget.Normalize();

        if (distanceToTarget < 8.f) {
            mPath.erase(mPath.begin());
            mPathTimer = AIMovementComponent::mPathTolerance;
        }

        Vector2 desiredVelocity = toTarget * mFowardSpeed;
        rb->ApplyForce(Vector2(desiredVelocity.x, 0.f));

        if (-toTarget.y > jumpThreshold)
        {
            Jump();
            break;
        }

        if (mPath.size() > 1) {
            Vector2 nextTarget = mPath[1];
            Vector2 toNextTargetFromTarget = nextTarget - targetPos;
            Vector2 toNextTarget = nextTarget - ownerCenter;
            
            if (-toNextTargetFromTarget.y > jumpThreshold && 
                Math::Abs(toNextTarget.x) < Game::TILE_SIZE * .8f)
            {
                Jump();
                break;
            }
        }

        break;
    }
    default:
        SetMovementState(MovementState::Wandering);
        break;
    }
        
}

void AIMovementComponent::Jump()
{   
    RigidBodyComponent* rb = GetOwnerRigidBody();
    if (!rb) return;

    if (rb->GetOnGround())
    {
        float xSpeed = rb->GetVelocity().x;
        float yForce = rb->GetVerticalForce(mJumpForceInBlocks);
        rb->ApplyForce(Vector2(xSpeed, yForce));
        SetMovementState(MovementState::Jumping);
    }
}

bool AIMovementComponent::CrazyDecision()
{
    float randomValue = Math::RandRange(MIN_CRAZINESS, MAX_CRAZINESS);
    return (randomValue <= mCraziness);
}

bool AIMovementComponent::CrazyDecision(float modifier)
{
    float randomValue = Math::RandRange(MIN_CRAZINESS, MAX_CRAZINESS);
    return (randomValue <= (mCraziness * modifier));
}

void AIMovementComponent::Update(float deltaTime)
{
    if(mOwner->GetBehaviorState() != BehaviorState::Moving)
        return;

    mInteligence += deltaTime * 0.01;
    if (mInteligence > 1.f)
    {
        mInteligence = 0.f;
    }

    if (CrazyDecision()) mInteligence = Math::RandRange(0.f, 1.f);

    if (GetMovementState() == MovementState::FollowingPath)
    {
        mPathTimer -= deltaTime;
        if (mPathTimer <= 0.f)
        {
            SDL_Log("AIMovementComponent::Update: Path timer expired, abandoning path.");
            SetMovementState(MovementState::Wandering);
            mPath.clear();
        }
    }
    
    Sense(deltaTime);
    Plan(deltaTime);
    Act(deltaTime);
}

void AIMovementComponent::SeekPlayer()
{
    if (MovementState::FollowingPath == GetMovementState())
        return;

    SetMovementState(MovementState::FollowingPath);
    
    Vector2 targetCenter = mOwner->GetGame()->GetZoe()->GetCenter();        
    SpatialHashing* sh = mOwner->GetGame()->GetSpatialHashing();
    mPath = sh->GetPath(mOwner, targetCenter);

    mPathTimer = AIMovementComponent::mPathTolerance;
}

void AIMovementComponent::LogState()
{
    switch (GetMovementState())
    {
    case MovementState::Wandering:
        SDL_Log("AIMovementComponent::LogState: Wandering");
        break;
    case MovementState::Jumping:
        SDL_Log("AIMovementComponent::LogState: Jumping");
        break;
    case MovementState::FollowingPath:
        SDL_Log("AIMovementComponent::LogState: FollowingPath");
        break;
    default:
        SDL_Log("AIMovementComponent::LogState: Unknown State");
        break;
    }
}