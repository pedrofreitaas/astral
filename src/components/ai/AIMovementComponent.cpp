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
    int jumpForceInBlocks, TypeOfMovement typeOfMovement,
    float pathTolerance, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering),
      mPreviousMovementState(MovementState::Wandering),
      mJumpForceInBlocks(jumpForceInBlocks), mInteligence(0.0f),
      mCraziness(craziness), mFowardSpeed(fowardSpeed),
      mPathTolerance(pathTolerance), mTypeOfMovement(typeOfMovement),
      mPathTimer(0.f), mLastSeenPlayerCenter(Vector2::Zero)
{
    if (mOwner->GetComponent<RigidBodyComponent>() == nullptr)
    {
        throw std::runtime_error("AIMovementComponent::AIMovementComponent: Owner missing RigidBodyComponent");
    }

    if (mCraziness > MAX_CRAZINESS)
        mCraziness = MAX_CRAZINESS;
    if (mCraziness < MIN_CRAZINESS)
        mCraziness = MIN_CRAZINESS;

    mOwnerEnemy = dynamic_cast<class Enemy*>(owner);
}

void AIMovementComponent::Sense(float deltaTime)
{
    if (mOwnerEnemy->PlayerOnFov(50.f, 300.f)) {
        mLastSeenPlayerCenter = mOwner->GetGame()->GetZoe()->GetCenter();
    }
}

void AIMovementComponent::Plan(float deltaTime)
{
    if (mPath.empty())
        return;
    
    SDL_Rect whereIThoughPlayerWas = mPath.back();
    Vector2 whereIThoughPlayerWasVec = Vector2(
        whereIThoughPlayerWas.x + whereIThoughPlayerWas.w*.5f, 
        whereIThoughPlayerWas.y + whereIThoughPlayerWas.h*.5f
    );

    Vector2 dist = mLastSeenPlayerCenter - whereIThoughPlayerWasVec;
    float distSq = dist.LengthSq();

    if (distSq >= 10000.f)
    {
        mPath.clear();
        mMovementState = MovementState::Wandering;
        SeekPlayer();
    }
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
        Vector2 ownerCenter = mOwner->GetCenter();
        float closestDistance = std::numeric_limits<float>::max();
        
        for (size_t i = 0; i < mPath.size(); ++i)
        {
            SDL_Rect targetRect = mPath[i];
            Vector2 targetPos = Vector2(
                static_cast<float>(targetRect.x + targetRect.w / 2),
                static_cast<float>(targetRect.y + targetRect.h / 2));

            float distance = (targetPos - ownerCenter).LengthSq();
            
            if (distance < closestDistance)
            {
                closestDistance = distance;
                currentIdx = static_cast<int>(i);
            }
        }

        return currentIdx;
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

    int targetIndex = GetTargetIndex();
    
    if (targetIndex == -1) {
        SetMovementState(MovementState::Wandering);
        mPath.clear();
        return;
    }

    if (targetIndex >= static_cast<int>(mPath.size())-1)
    {
        SetMovementState(MovementState::Wandering);
        mPath.clear();
        return;
    }

    SDL_Rect target = mPath[targetIndex];

    Vector2 targetPos = Vector2(
        static_cast<float>(target.x + target.w / 2),
        static_cast<float>(target.y + target.h / 2));
    Vector2 ownerCenter = mOwner->GetCenter();
    Vector2 toTarget = targetPos - ownerCenter;

    toTarget.Normalize();

    float fowardSpeedAbs = std::abs(mFowardSpeed);
    Vector2 force = toTarget * fowardSpeedAbs;
    rb->ApplyForce(force);
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
        float fowardSpeedAbs = std::abs(mFowardSpeed);
        Vector2 desiredHORVelocity = Vector2(resultantForce.x * fowardSpeedAbs, 0.f);
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
        if (!rb->GetOnGround() && rb->GetApplyGravity())
            break;

        if (mInteligence >= .5f)
            rb->ApplyForce(Vector2(mFowardSpeed, 0.f));
        else
            rb->ApplyForce(Vector2(-mFowardSpeed, 0.f));

        if (rb->GetApplyGravity() && CrazyDecision(.5f))
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
        float yForce = rb->GetVerticalVelY(mJumpForceInBlocks);
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
    if (GetMovementState() == MovementState::FollowingPath)
        return;

    SpatialHashing *sh = mOwner->GetGame()->GetSpatialHashing();
    
    mPath = sh->GetPath(
        mOwner, 
        mLastSeenPlayerCenter, 
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

void AIMovementComponent::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Blocks && GetMovementState() != MovementState::FollowingPath)
    {
        SetFowardSpeed(-GetFowardSpeed());
    }
}

void AIMovementComponent::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void AIMovementComponent::ApplyForce(const Vector2 &force)
{
    RigidBodyComponent *rb = GetOwnerRigidBody();
    if (rb)
    {
        rb->ApplyForce(force);
    }
}

void AIMovementComponent::BoostToPlayer(float intensity)
{
    RigidBodyComponent *rb = GetOwnerRigidBody();
    if (!rb)
        return;

    Vector2 boostDirection = mLastSeenPlayerCenter - mOwner->GetCenter();
    boostDirection.Normalize();

    Vector2 boostForce = boostDirection * intensity;
    rb->ApplyForce(boostForce);
}