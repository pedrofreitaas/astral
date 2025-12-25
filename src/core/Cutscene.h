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
    explicit Step(class Game* game, float maxTime=5.f) : mGame(game), mTimer(maxTime) {}
    virtual ~Step() = default;
    virtual void Update(float deltaTime) {
        mTimer -= deltaTime;
        if (mTimer <= 0.f) {
            SetComplete();
        }
    };
    virtual void PreUpdate() = 0;

    bool operator==(const Step& other) const { return this == &other; }
    bool GetIsComplete() const { return mIsComplete; }

protected:
    void SetComplete(bool v = true) { mIsComplete = v; }
    bool mIsComplete = false;
    Game *mGame;
    float mTimer;
};

class SpawnStep : public Step {
public:
    enum class ActorType {
        Star,
        // Add other actor types here as needed
    };
    SpawnStep(class Game* game, ActorType actorType, const Vector2& position, float maxTime=2.f): Step(game, maxTime), mActorType(actorType), mPosition(position) {};
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
        float speed=450.f,
        bool spin=false,
        float maxTime=5.f);

    void Update(float deltaTime) override;
    void PreUpdate() override;
private:
    float mSpeed;
    std::function<Actor*()> mGetTargetActor;
    std::function<Vector2()> mGetTargetPos;
    Vector2 mTargetPos;
    bool mSpin;
    float mSpinAngle;
};

class UnspawnStep : public Step {
public:
    UnspawnStep(class Game* game, std::function<Actor*()> targetActorFunc, float maxTime=2.f) 
    : Step(game, maxTime), mGetTargetActor(std::move(targetActorFunc)) {}
    UnspawnStep(class Game* game, Actor* targetActor, float maxTime=2.f)
    : Step(game, maxTime) {
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
    WaitStep(class Game* game, float duration) : Step(game, duration) {}
    void PreUpdate() override {};
};

class SoundStep : public Step {
public:
    SoundStep(class Game* game, std::string soundFile, bool loop = false, float maxTime=5.f) 
        : Step(game, maxTime), mSoundFile(soundFile), mLoop(loop) {}
    
    void Update(float deltaTime) override;
    void PreUpdate() override {};

private:
    std::string mSoundFile;
    bool mLoop;
};

class DialogueStep : public Step {
public:
    DialogueStep(class Game* game, std::vector<std::string>& messages) 
        : Step(game, -1.f), mMessages(messages), mSpeaker("") {};
    DialogueStep(class Game* game, const std::string& speaker, std::vector<std::string>& messages) 
        : Step(game, -1.f), mMessages(messages), mSpeaker(speaker) {};
    
    void Initialize();
    void Update(float deltaTime) override {
        if (GetIsComplete())
            return;
        
        Initialize();
    };
    void PreUpdate() override {};

private:
    std::vector<std::string> mMessages;
    std::string mSpeaker;
};

class Cutscene {
public:
    enum class State { Playing, Paused, Finished};

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
    void Finish();
    
    std::vector<std::unique_ptr<Step>> mSteps;
    State mState;
    size_t mCurrentStepIdx;
    bool mIsComplete;
    std::function<void()> mOnCompleteCallback;
    Game *mGame;
};
