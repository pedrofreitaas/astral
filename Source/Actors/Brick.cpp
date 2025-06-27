//
// Created by Pedro Oliveira May 18th, 2025.
//

#include "Brick.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"
#include "../Components/ColliderComponents/AABBColliderComponent.h"

float SHAKE_SPEED = 1.55f;

Brick::Brick(Game *game, const std::string &texturePath)
    : Block(game,texturePath,ColliderLayer::Bricks)
    , mIsShaking(false)
    , mShakeTime(0.0f)
{
}

void Brick::OnUpdate(float deltaTime) {
    if (!mIsShaking) return;

    float deltaY = (mYBeforeShaking - GetPosition().y) * deltaTime;
    deltaY = deltaY > 0.0f ? deltaY : SHAKE_SPEED;
    SetPosition(Vector2(GetPosition().x, GetPosition().y - deltaY));

    mShakeTime -= deltaTime;

    if (mShakeTime > 0.0f) return;
    
    mIsShaking = false;
    SetPosition(Vector2(GetPosition().x, mYBeforeShaking));
}

void Brick::OnColision() {
    if (mIsShaking) return;

    mIsShaking = true;
    mShakeTime = 0.3f;
    mYBeforeShaking = GetPosition().y;
}