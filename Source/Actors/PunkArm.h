//
// Created by Pedro Oliveira on 01/07/2025
//

#pragma once

#include "Actor.h"
#include "Punk.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"

class PunkArm : public Actor
{
public:
    explicit PunkArm(Game* game, class Punk* punk);

private:
    class DrawSpriteComponent* mDrawComponent;
    class Punk* mPunk;
};