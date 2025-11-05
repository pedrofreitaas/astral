#pragma once
#include <vector>
#include <functional>
#include "./Component.h"

class TimerComponent : public Component
{
public:
    TimerComponent(class Actor* owner) : Component(owner) {}
    
    void Update(float deltaTime) override {
        Component::Update(deltaTime);
        
        for (size_t i = 0; i < mDurations.size(); ++i) {
            mDurations[i] -= deltaTime;
            
            if (mDurations[i] <= 0.f) {
                if (mCallbacks[i]) {
                    mCallbacks[i]();
                }
            }
        }

        // Remove expired timers
        for (int i = static_cast<int>(mDurations.size()) - 1; i >= 0; --i) {
            if (mDurations[i] <= 0.f) {
                mDurations.erase(mDurations.begin() + i);
                mCallbacks.erase(mCallbacks.begin() + i);
            }
        }
    }

    void AddTimer(float duration, std::function<void()> callback) {
        mDurations.push_back(duration);
        mCallbacks.push_back(callback);
    }

protected:
    std::vector<float> mDurations;
    std::vector<std::function<void()>> mCallbacks;
};
