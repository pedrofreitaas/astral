#pragma once
#include <SDL.h>
#include <vector>
#include <memory>
#include <functional>
#include "../libs/Math.h"
#include "../actors/Actor.h"
#include "../components/RigidBodyComponent.h"
#include "../actors/Button.h"

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
    virtual void OnProcessInput(const std::vector<SDL_Event>& events) {};

    bool operator==(const Step& other) const { return this == &other; }
    bool GetIsComplete() const { return mIsComplete; }

protected:
    virtual void SetComplete(bool v = true) { mIsComplete = v; }
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

class UIAnimation;

class SpawnJoystickButtonStep : public Step {
public:
    SpawnJoystickButtonStep(class Game* game, Button button) : Step(game, 60.f), mButton(button), mAnimation(nullptr) {}
    void PreUpdate() override {};
    void Update(float deltaTime) override;
    UIAnimation* GetAnimation() const { return mAnimation; }
    void OnProcessInput(const std::vector<SDL_Event>& events) override;

private:
    Button mButton;
    UIAnimation* mAnimation;
};

class LaunchFireballStep : public Step {
public:
    LaunchFireballStep(class Game* game, float maxTime=2.f) : Step(game, maxTime) {}
    void PreUpdate() override {};
    void Update(float deltaTime) override;
};

class FireNevascaStep : public Step {
public:
    FireNevascaStep(class Game* game, float duration = 2.f) : Step(game, duration) {}
    void PreUpdate() override;
    void Update(float deltaTime) override;
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

class JumpStep : public Step {
public:
    JumpStep(class Game* game) : Step(game, .5f) {}
    void PreUpdate() override;
    void SetComplete(bool v) override;
};

class VentaniaStep : public Step {
public:
    VentaniaStep(class Game* game) : Step(game, .1f) {}
    void PreUpdate() override;
    void SetComplete(bool v) override;
};

class ShakeStep : public Step {
public:
    ShakeStep(class Game* game, float duration, float intensity = 3.f)
        : Step(game, 0.01f), mDuration(duration), mIntensity(intensity) {}
    void PreUpdate() override {};
    void Update(float deltaTime) override;
private:
    float mDuration;
    float mIntensity;
};

class FreezePhysicsStep : public Step {
public:
    FreezePhysicsStep(class Game* game) : Step(game, .01f) {}
    void PreUpdate() override;
};

class UnfreezePhysicsStep : public Step {
public:
    UnfreezePhysicsStep(class Game* game) : Step(game, .01f) {}
    void PreUpdate() override;
};

class DodgeStep : public Step {
public:
    DodgeStep(class Game* game) : Step(game, .01f) {}
    void PreUpdate() override;
    void SetComplete(bool v) override;
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
    void OnProcessInput(const std::vector<SDL_Event>& events);
    State GetState() const { return mState; }

private:
    void Finish();
    
    std::vector<std::unique_ptr<Step>> mSteps;
    State mState;
    size_t mCurrentStepIdx;
    bool mIsComplete;
    std::function<void()> mOnCompleteCallback;
    Game *mGame;
};
