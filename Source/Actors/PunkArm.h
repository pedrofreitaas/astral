//
// Created by Pedro Oliveira on 01/07/2025
//

#pragma once

#include "Actor.h"
#include "Punk.h"

class PunkArm : public Actor
{
public:
    explicit PunkArm(Game* game, Punk* punk);

private:
    class DrawComponent* mDrawComponent;
    class Punk* mPunk;
};