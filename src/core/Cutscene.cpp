#include "Cutscene.h"
#include <algorithm>
#include <utility>

Cutscene::Cutscene(std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback)
    : mSteps(std::move(steps)), mState(State::Paused), mCurrentStepIdx(0), 
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
            mIsComplete = true;
            if (mOnCompleteCallback) mOnCompleteCallback();
        }
    }
}
