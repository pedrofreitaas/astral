#include "AIMovementComponent.h"
#include "../../actors/Actor.h"
#include "../../actors/Zoe.h"
#include "../../core/Game.h"
#include "../../core/SpatialHashing.h"

const float MAX_CRAZINESS = 1.f;
const float MIN_CRAZINESS = 0.f;

AIMovementComponent::AIMovementComponent(
    class Actor* owner, float fowardSpeed,
    int jumpForceInBlocks,  float pathTolerance, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering), 
      mPreviousMovementState(MovementState::Wandering),
      mJumpForceInBlocks(jumpForceInBlocks), mInteligence(0.0f), 
      mCraziness(craziness), mFowardSpeed(fowardSpeed),
      mPathTolerance(pathTolerance)
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
    case MovementState::FollowingPathJumping:
        break;
    default:
        break;
    }
}

void AIMovementComponent::Act(float deltaTime)
{
    RigidBodyComponent* rb = GetOwnerRigidBody();
    AABBColliderComponent* collider = GetOwnerCollider();

    if (!collider || !rb) {
        throw std::runtime_error("AIMovementComponent::Act: Owner missing RigidBodyComponent or AABBColliderComponent");
    }

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

    case MovementState::FollowingPathJumping:
        if (rb->GetOnGround())
        {
            SetMovementState(MovementState::FollowingPath);
        }
        break;
    
    case MovementState::FollowingPath: {
        if (!rb->GetOnGround()) break;

        int currentIdx = -1;
        
        for (size_t i = 0; i < mPath.size(); ++i) {
            if (collider->IsCollidingRect(mPath[i])) {
                currentIdx = i;
                break;
            }
        }

        SDL_Rect target;
        if (currentIdx == -1) {
            target = mPath[0]; // need to do this more intelligently in the future.
        } else if (currentIdx + 1 < static_cast<int>(mPath.size())) {
            target = mPath[currentIdx + 1];
        } else {
            // Reached the end of the path
            SetMovementState(MovementState::Wandering);
            mPath.clear();
            break;
        }

        Vector2 targetPos = Vector2(
            static_cast<float>(target.x + target.w / 2),
            static_cast<float>(target.y + target.h / 2)
        );
        Vector2 ownerCenter = mOwner->GetCenter();
        Vector2 toTarget = targetPos - ownerCenter;

        float jumpThreshold = Game::TILE_SIZE * 0.75f;
        
        float distanceToTarget = toTarget.Length();

        toTarget.Normalize();

        Vector2 desiredVelocity = toTarget * mFowardSpeed;
        rb->ApplyForce(Vector2(desiredVelocity.x, 0.f));

        if (-toTarget.y > jumpThreshold)
        {
            Jump(true);
            break;
        }

        if (currentIdx + 2 < static_cast<int>(mPath.size())) {
            SDL_Rect nextRect = mPath[currentIdx + 2];
            Vector2 nextTarget = Vector2(
                static_cast<float>(nextRect.x + nextRect.w / 2),
                static_cast<float>(nextRect.y + nextRect.h / 2)
            );
            Vector2 toNextTargetFromTarget = nextTarget - targetPos;
            Vector2 toNextTarget = nextTarget - ownerCenter;
            
            if (-toNextTargetFromTarget.y > jumpThreshold && 
                Math::Abs(toNextTarget.x) < Game::TILE_SIZE * .8f)
            {
                Jump(true);
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

void AIMovementComponent::Jump(bool isFollowingPath)
{   
    RigidBodyComponent* rb = GetOwnerRigidBody();
    if (!rb) return;

    if (rb->GetOnGround())
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
    if(mOwner->GetBehaviorState() != BehaviorState::Moving)
        return;

    mInteligence += deltaTime * 0.01;
    if (mInteligence > 1.f)
    {
        mInteligence = 0.f;
    }

    if (CrazyDecision()) mInteligence = Math::RandRange(0.f, 1.f);

    if (GetMovementState() == MovementState::FollowingPath ||
        GetMovementState() == MovementState::FollowingPathJumping)
    {
        mPathTimer -= deltaTime;
        SDL_Log("AIMovementComponent::Update: Path timer: %f", mPathTimer);
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

    if (mPath.empty())
    {
        SDL_Log("AIMovementComponent::SeekPlayer: No path found to player.");
        SetMovementState(MovementState::Wandering);
        return;
    }

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