#pragma once

#include "Actor.h"
#include "../components/draw/DrawComponent.h"
#include <string>

class Tile : public Actor
{
private:

public:
    Tile(
        Game *game, 
        SDL_Texture* tilesetTexture,
        const Vector2& worldPosition,
        const Vector2& tilesetPosition,
        int width, int height,
        int boundBoxWidth, int boundBoxHeight,
        int boundBoxOffsetX=0, int boundBoxOffsetY=0,
        const DrawLayerPosition &layer=DrawLayerPosition::Ground
    );
};