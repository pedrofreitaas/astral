#pragma once
#include <SDL.h>
#include <vector>
#include <memory>
#include <functional>

class Step {
public:
    virtual ~Step() = default;
    virtual void Update(float deltaTime) = 0;

    bool operator==(const Step& other) const { return this == &other; }
    bool GetIsComplete() const { return mIsComplete; }

protected:
    void SetComplete(bool v = true) { mIsComplete = v; }

private:
    bool mIsComplete = false;
};

class Cutscene {
public:
    enum class State { Playing, Paused };

    explicit Cutscene(std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback);
    ~Cutscene() = default;

    void AddStep(std::unique_ptr<Step> step);
    void AddSteps(std::vector<std::unique_ptr<Step>> steps);
    void RemoveStep(Step& step);
    void Play();
    void Pause();
    void Update(float deltaTime);
    bool IsComplete() const { return mIsComplete; }

private:
    std::vector<std::unique_ptr<Step>> mSteps;
    State mState;
    size_t mCurrentStepIdx;
    bool mIsComplete;
    std::function<void()> mOnCompleteCallback;
};
