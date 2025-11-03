#include "AIMovementComponent.h"
#include "../../actors/Actor.h"
#include "../../actors/Zoe.h"
#include "../../core/Game.h"
#include "../../core/SpatialHashing.h"

const float MAX_CRAZINESS = 1.f;
const float MIN_CRAZINESS = 0.f;

AIMovementComponent::AIMovementComponent(
    class Actor *owner, float fowardSpeed,
    int jumpForceInBlocks, TypeOfMovement typeOfMovement,
    float pathTolerance, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering),
      mPreviousMovementState(MovementState::Wandering),
      mJumpForceInBlocks(jumpForceInBlocks), mInteligence(0.0f),
      mCraziness(craziness), mFowardSpeed(fowardSpeed),
      mPathTolerance(pathTolerance), mTypeOfMovement(typeOfMovement)
{
    if (mCraziness > MAX_CRAZINESS)
        mCraziness = MAX_CRAZINESS;
    if (mCraziness < MIN_CRAZINESS)
        mCraziness = MIN_CRAZINESS;
}

void AIMovementComponent::Sense(float deltaTime)
{
}

void AIMovementComponent::Plan(float deltaTime)
{
}

int AIMovementComponent::GetTargetIndex()
{
    AABBColliderComponent *collider = GetOwnerCollider();

    if (mPath.empty() || !collider)
        return -1;

    int currentIdx = -1;
    for (size_t i = 0; i < mPath.size(); ++i)
    {
        if (collider->IsCollidingRect(mPath[i]))
        {
            currentIdx = i;
            break;
        }
    }
    
    // npc not on path, let's find the closest rect in path, and set as target.
    if (currentIdx == -1) 
    {
        int closestRectIdx = -1;
        float closestDistSq = std::numeric_limits<float>::max();
        Vector2 ownerCenter = mOwner->GetCenter();

        for (size_t i = 0; i < mPath.size(); ++i)
        {
            SDL_Rect rect = mPath[i];
            Vector2 rectCenter = Vector2(
                static_cast<float>(rect.x + rect.w / 2),
                static_cast<float>(rect.y + rect.h / 2));
            float distSq = (rectCenter - ownerCenter).LengthSq();
            if (distSq < closestDistSq)
            {
                closestDistSq = distSq;
                closestRectIdx = static_cast<int>(i);
            }
        }

        return closestRectIdx;
    }
    
    // npc on path, follow to next rect in path.
    if (currentIdx + 1 < static_cast<int>(mPath.size())) 
    {
        return currentIdx + 1;
    }

    return mPath.size() - 1;
}

void AIMovementComponent::FollowPathFlier()
{
    AABBColliderComponent *collider = GetOwnerCollider();
    RigidBodyComponent *rb = GetOwnerRigidBody();

    int currentIdx = -1;

    for (size_t i = 0; i < mPath.size(); ++i)
    {
        if (collider->IsCollidingRect(mPath[i]))
        {
            currentIdx = i;
            break;
        }
    }

    SDL_Rect target;
    if (currentIdx == -1)
    {
        target = mPath[0]; // need to do this more intelligently in the future.
    }
    else if (currentIdx + 1 < static_cast<int>(mPath.size()))
    {
        target = mPath[currentIdx + 1];
    }
    else
    {
        // Reached the end of the path
        SetMovementState(MovementState::Wandering);
        mPath.clear();
        return;
    }

    Vector2 targetPos = Vector2(
        static_cast<float>(target.x + target.w / 2),
        static_cast<float>(target.y + target.h / 2));
    Vector2 ownerCenter = mOwner->GetCenter();
    Vector2 toTarget = targetPos - ownerCenter;

    float distanceToTarget = toTarget.Length();

    toTarget.Normalize();

    Vector2 desiredVelocity = toTarget * mFowardSpeed;
    rb->ApplyForce(desiredVelocity);
}

void AIMovementComponent::FollowPathWalker()
{
    AABBColliderComponent *collider = GetOwnerCollider();
    RigidBodyComponent *rb = GetOwnerRigidBody();

    if (!rb->GetOnGround())
        return;

    int targetIndex = GetTargetIndex();

    if (targetIndex == -1) {
        mPath.clear();
        SetMovementState(MovementState::Wandering);
        return;
    }

    // npc reached end the last rect, end of path.
    if (targetIndex >= static_cast<int>(mPath.size()) - 1)
    {
        SetMovementState(MovementState::Wandering);
        mPath.clear();
        return;
    }

    Vector2 ownerCenter = mOwner->GetCenter();
    
    // resultant force of the next 3 steps, if available.
    Vector2 resultantForce = Vector2::Zero;
    int stepsToConsider = 3;
    for (int i = 0; i < stepsToConsider; ++i)
    {
        int idx = targetIndex + i;
        if (idx >= static_cast<int>(mPath.size()))
            break;
        
        SDL_Rect targetRect = mPath[idx];
        Vector2 targetPos = Vector2(
            static_cast<float>(targetRect.x + targetRect.w / 2),
            static_cast<float>(targetRect.y + targetRect.h / 2));
        
        
        Vector2 toTarget = targetPos - ownerCenter;
        
        toTarget.Normalize();
        resultantForce += toTarget;
        resultantForce *= (1.f / static_cast<float>(stepsToConsider));
    }

    if (rb->GetOnGround()) {
        Vector2 desiredHORVelocity = Vector2(resultantForce.x * mFowardSpeed, 0.f);
        rb->ApplyForce(desiredHORVelocity);
    }

    if (resultantForce.y < -0.3f)
    {
        Jump(true);
    }
}

void AIMovementComponent::Act(float deltaTime)
{
    RigidBodyComponent *rb = GetOwnerRigidBody();
    AABBColliderComponent *collider = GetOwnerCollider();

    if (!collider || !rb)
    {
        throw std::runtime_error("AIMovementComponent::Act: Owner missing RigidBodyComponent or AABBColliderComponent");
    }

    switch (GetMovementState())
    {
    case MovementState::Wandering:
        if (!rb->GetOnGround())
            break;

        if (mInteligence >= .5f)
            rb->ApplyForce(Vector2(mFowardSpeed, 0.f));
        else
            rb->ApplyForce(Vector2(-mFowardSpeed, 0.f));

        if (CrazyDecision(.5f))
            Jump();

        break;

    case MovementState::Jumping:
        if (rb->GetOnGround())
        {
            SetMovementState(mPreviousMovementState);
        }
        break;

    case MovementState::FollowingPathJumping:
        if (rb->GetOnGround())
        {
            SetMovementState(MovementState::FollowingPath);
        }
        break;

    case MovementState::FollowingPath:
    {
        if(mTypeOfMovement == TypeOfMovement::Walker) 
            FollowPathWalker();
        else 
            FollowPathFlier();

        break;
    }

    default:
        SetMovementState(MovementState::Wandering);
        break;
    }
}

void AIMovementComponent::Jump(bool isFollowingPath)
{
    RigidBodyComponent *rb = GetOwnerRigidBody();
    if (!rb)
        return;

    if (rb->GetOnGround() && mTypeOfMovement == TypeOfMovement::Walker)
    {
        float xSpeed = rb->GetVelocity().x;
        float yForce = rb->GetVerticalForce(mJumpForceInBlocks);
        rb->ApplyForce(Vector2(xSpeed, yForce));
        SetMovementState(
            isFollowingPath ? MovementState::FollowingPathJumping : MovementState::Jumping);
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
    if (mOwner->GetBehaviorState() != BehaviorState::Moving)
        return;

    mInteligence += deltaTime * 0.01;
    if (mInteligence > 1.f)
    {
        mInteligence = 0.f;
    }

    if (CrazyDecision())
        mInteligence = Math::RandRange(0.f, 1.f);

    if (GetMovementState() == MovementState::FollowingPath ||
        GetMovementState() == MovementState::FollowingPathJumping)
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

    Vector2 targetCenter = mOwner->GetGame()->GetZoe()->GetCenter();
    SpatialHashing *sh = mOwner->GetGame()->GetSpatialHashing();
    mPath = sh->GetPath(
        mOwner, 
        targetCenter, 
        mTypeOfMovement == TypeOfMovement::Flier);

    if (mPath.empty())
    {
        return;
    }

    SetMovementState(MovementState::FollowingPath);
    mPathTimer = mPathTolerance;
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
    case MovementState::FollowingPathJumping:
        SDL_Log("AIMovementComponent::LogState: FollowingPathJumping");
        break;
    case MovementState::FollowingPath:
        SDL_Log("AIMovementComponent::LogState: FollowingPath");
        break;
    default:
        SDL_Log("AIMovementComponent::LogState: Unknown State");
        break;
    }
}