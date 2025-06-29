//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "Actor.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include <string>

class Block : public Actor
{
public:
    Block(Game *game, const std::string &texturePath, const DrawLayerPosition &layer);

    virtual void OnColision() {};
};