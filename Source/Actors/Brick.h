//
// Created by Pedro Oliveira May 18th, 2025.
//

#pragma once

#include "Block.h"
#include <string>

class Brick : public Block
{
public:
    explicit Brick(Game* game, const std::string &texturePath);
    void OnColision() override;

private:
    void OnUpdate(float deltaTime) override;

    bool mIsShaking;
    float mShakeTime;
    float mYBeforeShaking;
};