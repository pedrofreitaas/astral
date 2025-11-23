#include <algorithm>
#include <utility>
#include <stdexcept>
#include "Cutscene.h"
#include "Game.h"
#include "../actors/Star.h"
#include "../ui/DialogueSystem.h"

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
    if (mSteps.empty()) {
        throw std::runtime_error("Cannot play a cutscene with no steps.");
    }
    
    mState = State::Playing;
    mGame->SetGamePlayState(Game::GamePlayState::PlayingCutscene);
    
    mSteps[mCurrentStepIdx]->PreUpdate();
}

void Cutscene::Pause() { 
    mState = State::Paused;  
}

void Cutscene::Update(float deltaTime) {
    if (mIsComplete) {
        return;
    }

    if (mState != State::Playing || mSteps.empty()) {
        SDL_Log("Updating a cutscene not valid for updates");
        return;
    }

    const std::unique_ptr<Step>& cur = mSteps[mCurrentStepIdx];
    cur->Update(deltaTime);

    if (cur->GetIsComplete()) {
        mCurrentStepIdx++;

        if (mCurrentStepIdx >= mSteps.size()) {
            Finish();
            return;
        }

        mSteps[mCurrentStepIdx]->PreUpdate();
    }
}

void Cutscene::Finish() {
    mState = State::Finished;
    mGame->SetGamePlayState(Game::GamePlayState::Playing);
    mIsComplete = true;

    if (mOnCompleteCallback) 
    {
        mOnCompleteCallback();
    }
}