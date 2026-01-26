#pragma once

#include <string>
#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawSpriteComponent.h"

enum class SnowDirection {
    RIGHT=0,
    LEFT=1,
    UP=2,
    DOWN=3,
};

class Snow : public Actor
{
private:

public:
    Snow(
        Game *game, 
        const Vector2& center,
        SnowDirection dir
    );

    void Unspawn();
};