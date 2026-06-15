#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../../components/TimerComponent.h"
#include "./ZathuraRock.h"
#include "../Collider.h"

enum class ZathuraAttacks
{
    Attack1,
    Attack2,
    Attack3,
    Rocks,
    None
};

class Zathura : public Enemy
{
public:
    explicit Zathura(Game* game, const Vector2& position);

    void ManageState() override;
    void AnimationEndCallback(std::string animationName) override;
    void ManageAnimations() override;

    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void PlayBlockedPlayerSound();

    bool GetIsWaitingToThrowRocks() { return mIsWaitingToThrowRocks; }
    void SetIsWaitingToThrowRocks(bool value) { mIsWaitingToThrowRocks = value; }

    void DeathCutscenePlayedCallback();

protected:
    void SetBehaviorState(BehaviorState newState) override;

private:
    ZathuraAttacks mCurrentAttack;
    SoundHandle mBlockedPlayerSoundHandle;
    Timer* mRockAttackTimerHandle, *mAttack1CooldownTimer, *mAttack2CooldownTimer, *mAttack3CooldownTimer;

    bool mIsWaitingToThrowRocks, mSpawnedAttackCollider, mPreDeathCutscenePlayed;
};