#include "AIMovementComponent.h"
#include "../../actors/Actor.h"

const float MAX_CRAZINESS = 1.f;
const float MIN_CRAZINESS = 0.f;

AIMovementComponent::AIMovementComponent(class Actor* owner, float fowardSpeed, float craziness)
    : Component(owner), mMovementState(MovementState::Wandering),
      mJumpForceInBlocks(3), mInteligence(0.0f), mCraziness(craziness), mFowardSpeed(fowardSpeed)
{
    if (mCraziness > MAX_CRAZINESS) mCraziness = MAX_CRAZINESS;
    if (mCraziness < MIN_CRAZINESS) mCraziness = MIN_CRAZINESS;
}

void AIMovementComponent::Sense(float deltaTime)
{
    AABBColliderComponent* collider = GetOwnerCollider();
}

void AIMovementComponent::Plan(float deltaTime)
{
    switch (mMovementState)
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

    switch (mMovementState)
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
            mMovementState = MovementState::Wandering;
        }
        break;
    
    case MovementState::FollowingPath:
        SDL_Log("Following Path not implemented yet!");
        mMovementState = MovementState::Wandering;
        break;
    default:
        mMovementState = MovementState::Wandering;
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
        mMovementState = MovementState::Jumping;
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
    
    Sense(deltaTime);
    Plan(deltaTime);
    Act(deltaTime);
}