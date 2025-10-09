#include <algorithm>
#include <utility>
#include <stdexcept>
#include "Cutscene.h"
#include "Game.h"
#include "../actors/Star.h"
#include "../ui/DialogueSystem.h"

MoveStep::MoveStep(class Game* game, Actor* targetActor, const Vector2& targetPos, float speed)
    : Step(game), mTargetPos(targetPos), mSpeed(speed)
{
    mGetTargetActor = [targetActor]() { return targetActor; };
    
    if (mGetTargetActor()->GetComponent<RigidBodyComponent>() == nullptr) {
        throw std::invalid_argument("MoveStep requires the target Actor to have a RigidBodyComponent");
    }
}

void MoveStep::Update(float deltaTime) {
    if (GetIsComplete()) return;

    Actor* mTargetActor = mGetTargetActor();

    if (!mTargetActor) {
        throw std::runtime_error("MoveStep target Actor is null");
    }

    RigidBodyComponent* rb = mTargetActor->GetComponent<RigidBodyComponent>();
    if (!rb) {
        throw std::runtime_error("MoveStep target Actor lost its RigidBodyComponent");
    }

    Vector2 toTarget = mTargetPos - mTargetActor->GetPosition();
    float distanceToTarget = toTarget.Length();

    if (distanceToTarget < 1e-3f) {
        // Close enough to target
        rb->SetVelocity(Vector2::Zero);
        mTargetActor->SetPosition(mTargetPos);
        SetComplete();
        return;
    }

    Vector2 direction = toTarget;
    direction.Normalize();
    Vector2 desiredVelocity = direction * mSpeed;

    rb->SetVelocity(desiredVelocity);

    // Check if we would overshoot the target in this frame
    if (distanceToTarget < desiredVelocity.Length() * deltaTime) {
        // Snap to target and complete step
        rb->SetVelocity(Vector2::Zero);
        mTargetActor->SetPosition(mTargetPos);
        SetComplete();
    }
}

SpawnStep::SpawnStep(class Game* game, ActorType actorType, const Vector2& position)
    : Step(game), mActorType(actorType), mPosition(position)
{
}

void SpawnStep::Update(float deltaTime) {
    if (GetIsComplete()) return;

    Actor* newActor = nullptr;

    if (mActorType == ActorType::Star) {
        newActor = new Star(mGame);
    }

    if (newActor) {
        newActor->SetPosition(mPosition);
        SetComplete();
    }
    else {
        throw std::runtime_error("SpawnStep failed to create actor of type: " + 
                                  std::to_string(static_cast<int>(mActorType)));
    }
}

void DialogueStep::Update(float deltaTime) {
    if (GetIsComplete()) return;

    mGame->GetDialogueSystem()->StartDialogue(
        mMessages, 
        [this]() {
            SetComplete();
        }
    );
}

Cutscene::Cutscene(std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback, Game* game)
    : mSteps(std::move(steps)), mState(State::Paused), mCurrentStepIdx(0), mGame(game),
    mIsComplete(false), mOnCompleteCallback(std::move(onCompleteCallback))
{
}

void Cutscene::AddStep(std::unique_ptr<Step> step) {
    mSteps.push_back(std::move(step));
}

void Cutscene::AddSteps(std::vector<std::unique_ptr<Step>> steps) {
    for (auto& s : steps) mSteps.push_back(std::move(s));
}

void Cutscene::RemoveStep(Step& step) {
    auto it = std::find_if(
        mSteps.begin(), 
        mSteps.end(),
        [&step](const std::unique_ptr<Step>& p){ return p.get() == &step; }
    );
    
    if (it != mSteps.end()) 
        mSteps.erase(it);
}

void Cutscene::Play()  { 
    mState = State::Playing; 
}

void Cutscene::Pause() { 
    mState = State::Paused;  
}

void Cutscene::Update(float deltaTime) {
    if (mState != State::Playing || mIsComplete || mSteps.empty()) {
        SDL_Log("Updating a cutscene not valid for updates");
        return;
    }

    const std::unique_ptr<Step>& cur = mSteps[mCurrentStepIdx];
    cur->Update(deltaTime);

    if (cur->GetIsComplete()) {
        mCurrentStepIdx++;

        if (mCurrentStepIdx >= mSteps.size()) {
            mState = State::Paused;
            mGame->SetGamePlayState(Game::GamePlayState::Playing);
            mIsComplete = true;
            if (mOnCompleteCallback) mOnCompleteCallback();
        }
    }
}
