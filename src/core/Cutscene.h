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
    virtual void PreUpdate() = 0;

    bool operator==(const Step& other) const { return this == &other; }
    bool GetIsComplete() const { return mIsComplete; }

protected:
    void SetComplete(bool v = true) { mIsComplete = v; }
    bool mIsComplete = false;
    Game *mGame;
};

class SpawnStep : public Step {
public:
    enum class ActorType {
        Star,
        // Add other actor types here as needed
    };
    SpawnStep(class Game* game, ActorType actorType, const Vector2& position);
    void Update(float deltaTime) override;
    void PreUpdate() override {};
private:
    ActorType mActorType;
    Vector2 mPosition;
};

class MoveStep : public Step {
public:
    MoveStep(
        class Game* game, 
        std::function<Actor*()> targetActorFunc,
        std::function<Vector2()> getTargetPosFunc, 
        float speed);

    void Update(float deltaTime) override;
    void PreUpdate() override;
private:
    float mSpeed;
    std::function<Actor*()> mGetTargetActor;
    std::function<Vector2()> mGetTargetPos;
    Vector2 mTargetPos;
};

class UnspawnStep : public Step {
public:
    UnspawnStep(class Game* game, std::function<Actor*()> targetActorFunc) : Step(game), mGetTargetActor(std::move(targetActorFunc)) {}
    UnspawnStep(class Game* game, Actor* targetActor): Step(game) {
        mGetTargetActor = [targetActor]() { return targetActor; };
    }
    void Update(float deltaTime) override {
        if (GetIsComplete()) return;

        Actor* targetActor = mGetTargetActor();
        if (targetActor) {
            targetActor->SetState(ActorState::Destroy);
            SetComplete();
        } else {
            throw std::runtime_error("UnspawnStep target Actor is null");
        }
    }
    void PreUpdate() override {};
private:
    std::function<Actor*()> mGetTargetActor;
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
    void PreUpdate() override {};
private:
    float mDuration;
    float mElapsed;
};

class DialogueStep : public Step {
public:
    DialogueStep(class Game* game, std::vector<std::string>& messages) 
        : Step(game), mMessages(messages), mSpeaker("") {}
    DialogueStep(class Game* game, const std::string& speaker, std::vector<std::string>& messages) 
        : Step(game), mMessages(messages), mSpeaker(speaker) {}
    
    void Update(float deltaTime) override;
    void PreUpdate() override {};
private:
    std::vector<std::string> mMessages;
    std::string mSpeaker;
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
