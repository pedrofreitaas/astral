#include <algorithm>
#include <utility>
#include <stdexcept>
#include "Cutscene.h"
#include "Game.h"
#include "../actors/Star.h"
#include "../ui/DialogueSystem.h"

MoveStep::MoveStep(
    class Game* game, 
    std::function<Actor*()> targetActorFunc,
    std::function<Vector2()> getTargetPosFunc,
    float speed, bool spin)
    : 
    Step(game), mSpeed(speed), 
    mGetTargetActor(std::move(targetActorFunc)), 
    mGetTargetPos(std::move(getTargetPosFunc)),
    mTargetPos(Vector2::Zero), mSpinAngle(0.0f), mSpin(spin)
{
}

void MoveStep::PreUpdate()
{
    mTargetPos = mGetTargetPos();
    Actor *mTargetActor = mGetTargetActor();

    if (!mTargetActor)
    {
        throw std::runtime_error("MoveStep target Actor is null");
    }

    RigidBodyComponent *rb = mTargetActor->GetComponent<RigidBodyComponent>();
    if (!rb)
    {
        throw std::runtime_error("MoveStep target Actor lost its RigidBodyComponent");
    }
}

void MoveStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;
    
    Actor *mTargetActor = mGetTargetActor();    
    RigidBodyComponent *rb = mTargetActor->GetComponent<RigidBodyComponent>();
    
    // Get the center position of the actor
    Vector2 actorCenterOffset = mTargetActor->GetCenter() - mTargetActor->GetPosition();
    Vector2 actorCenter = mTargetActor->GetCenter();
    
    Vector2 toTarget = mTargetPos - actorCenter;
    float distanceToTarget = toTarget.Length();
    
    if (distanceToTarget < 1e-3f)
    {
        // Close enough to target
        rb->SetVelocity(Vector2::Zero);
        mTargetActor->SetPosition(mTargetPos - actorCenterOffset);
        SetComplete();
        return;
    }

    if (mSpin)
    {
        mSpinAngle += 360.f * deltaTime;
        if (mSpinAngle >= 360.f)
            mSpinAngle -= 360.f;
        mTargetActor->SetRotation(mSpinAngle);
    }

    Vector2 direction = toTarget;
    direction.Normalize();
    Vector2 desiredVelocity = direction * mSpeed;
    rb->SetVelocity(desiredVelocity);
    
    // Check if we would overshoot the target in this frame
    if (distanceToTarget < desiredVelocity.Length() * deltaTime)
    {
        // Snap to target and complete step
        rb->SetVelocity(Vector2::Zero);
        mTargetActor->SetPosition(mTargetPos - actorCenterOffset);
        SetComplete();
    }
    
    // check if movement resulted in getting out of the camera
    if (!mGame->ActorOnCamera(mTargetActor))
    {
        rb->SetVelocity(Vector2::Zero);
        SetComplete();
    }
}

SpawnStep::SpawnStep(class Game *game, ActorType actorType, const Vector2 &position)
    : Step(game), mActorType(actorType), mPosition(position)
{
}

void SpawnStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;

    Actor *newActor = nullptr;

    if (mActorType == ActorType::Star)
    {
        newActor = new Star(mGame);
    }

    if (newActor)
    {
        newActor->SetPosition(mPosition);
        SetComplete();
    }
    else
    {
        throw std::runtime_error("SpawnStep failed to create actor of type: " +
                                 std::to_string(static_cast<int>(mActorType)));
    }
}

void DialogueStep::Update(float deltaTime)
{
    if (GetIsComplete())
        return;

    if (!mSpeaker.empty())
    {
        mGame->GetDialogueSystem()->StartDialogueWithSpeaker(
            mSpeaker,
            mMessages,
            [this]()
            {
                SetComplete();
            });
    }
    else
    {
        mGame->GetDialogueSystem()->StartDialogue(
            mMessages,
            [this]()
            {
                SetComplete();
            });
    }
}
