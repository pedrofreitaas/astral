#pragma once
#include <vector>
#include <functional>
#include "./Component.h"

class Timer {
public:
    int id;
    float duration, elapsed;
    bool removable;
    std::function<void()> callback;

    Timer() : id(0), duration(0.f), elapsed(0.f), removable(false), callback(nullptr) {}
    Timer(int id, float dur, bool removable, std::function<void()> cb) : id(id), duration(dur), elapsed(0.f), removable(removable), callback(cb) {}
};

class TimerComponent : public Component
{
private:
    int nextTimerId = 0;

public:
    TimerComponent(class Actor* owner) : Component(owner) {}
    
    void Update(float deltaTime) override {
        Component::Update(deltaTime);
        
        for (size_t i = 0; i < mTimers.size(); ++i) {
            mTimers[i]->elapsed += deltaTime;
            
            if (mTimers[i]->elapsed >= mTimers[i]->duration) {
                if (mTimers[i]->callback) {
                    mTimers[i]->callback();
                }
            }
        }

        // Remove expired timers
        for (int i = static_cast<int>(mTimers.size()) - 1; i >= 0; --i) {
            if (mTimers[i]->elapsed >= mTimers[i]->duration && mTimers[i]->removable) {
                delete mTimers[i];
                mTimers.erase(mTimers.begin() + i);
            }
        }
    }

    Timer* AddTimer(float duration, std::function<void()> callback) {
        struct Timer* newTimer = new Timer(
            nextTimerId,
            duration,
            true,
            callback
        );

        nextTimerId++;

        mTimers.push_back(newTimer);

        return newTimer;
    }

    // take care to avoid memory overloads with this timer
    Timer *AddNotRemovableTimer(float duration, std::function<void()> callback) {
        struct Timer* newTimer = new Timer(
            nextTimerId,
            duration,
            false,
            callback
        );

        nextTimerId++;

        mTimers.push_back(newTimer);

        return newTimer;
    }

    float checkTimerRemaining(Timer* timer) {
        for (const auto& t : mTimers) {
            if (t->id == timer->id) {
                return t->duration - t->elapsed;
            }
        }
        return 0.f;
    }

    void Restart(Timer *timer) {
        for (const auto& t : mTimers) {
            if (t->id == timer->id) {
                t->elapsed = 0.f;
                return;
            }
        }
    }

protected:
    std::vector<Timer*> mTimers;
};
