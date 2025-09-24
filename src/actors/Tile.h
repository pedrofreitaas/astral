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
        int gridX, int gridY, 
        int height, int width,
        int boundBoxWidth, int boundBoxHeight,
        int boundBoxOffsetX=0, int boundBoxOffsetY=0,
        const DrawLayerPosition &layer=DrawLayerPosition::Ground
    );
};