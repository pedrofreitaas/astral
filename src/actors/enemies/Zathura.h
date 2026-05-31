#pragma once

#include <SDL.h>
#include "../Enemy.h"
#include "../../components/TimerComponent.h"

enum class ZathuraAttacks
{
    Attack1,
    Attack2,
    Attack3,
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

private:
    ZathuraAttacks mCurrentAttack;
    SoundHandle mBlockedPlayerSoundHandle;
};