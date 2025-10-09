#pragma once
#include <SDL.h>
#include <vector>
#include <memory>
#include <functional>
#include "../libs/Math.h"
#include "../actors/Actor.h"
#include "../components/RigidBodyComponent.h"

class Game;

class Step {
public:
    explicit Step(class Game* game) : mGame(game) {}
    virtual ~Step() = default;
    virtual void Update(float deltaTime) = 0;

    bool operator==(const Step& other) const { return this == &other; }
    bool GetIsComplete() const { return mIsComplete; }

protected:
    void SetComplete(bool v = true) { mIsComplete = v; }
    bool mIsComplete = false;
    Game *mGame;
};

class MoveStep : public Step {
public:
    MoveStep(class Game* game, std::function<Actor*()> targetActorFunc, const Vector2& targetPos, float speed):
        Step(game), mTargetPos(targetPos), mSpeed(speed), mGetTargetActor(std::move(targetActorFunc)) {};

    MoveStep(class Game* game, Actor* targetActor, const Vector2& targetPos, float speed);
    void Update(float deltaTime) override;
private:
    float mSpeed;
    Vector2 mTargetPos;
    std::function<Actor*()> mGetTargetActor;
};

class SpawnStep : public Step {
public:
    enum class ActorType {
        Star,
        // Add other actor types here as needed
    };
    SpawnStep(class Game* game, ActorType actorType, const Vector2& position);
    void Update(float deltaTime) override;
private:
    ActorType mActorType;
    Vector2 mPosition;
};

class WaitStep : public Step {
public:
    WaitStep(class Game* game, float duration) : Step(game), mDuration(duration), mElapsed(0.0f) {}
    void Update(float deltaTime) override {
        if (GetIsComplete()) return;
        mElapsed += deltaTime;
        if (mElapsed >= mDuration) {
            SetComplete();
        }
    }
private:
    float mDuration;
    float mElapsed;
};

class Cutscene {
public:
    enum class State { Playing, Paused };

    explicit Cutscene(std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback, Game* game);
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
    Game *mGame;
};
