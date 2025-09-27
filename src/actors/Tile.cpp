#include "Tile.h"
#include "../core/Game.h"
#include "../components/draw/DrawTileComponent.h"
#include "../components/collider/AABBColliderComponent.h"

Tile::Tile(
    Game *game, 
    SDL_Texture *tilesetTexture,
    int gridX, int gridY,
    int width, int height,
    int boundBoxWidth, int boundBoxHeight,
    int boundBoxOffsetX, int boundBoxOffsetY,
    const DrawLayerPosition &layer
) : Actor(game)
{
    if (!tilesetTexture)
    {
        throw std::runtime_error("Tileset texture is null");
        return;
    }

    mPosition.x = gridX * width;
    mPosition.y = gridY * height;

    new DrawTileComponent(
        this,
        tilesetTexture,
        gridX, gridY,
        width, height,
        static_cast<int>(layer)
    );

    bool hasCollision = (layer == DrawLayerPosition::Player);

    if (!hasCollision)
        return;

    new AABBColliderComponent(
        this, 
        boundBoxOffsetX, boundBoxOffsetY, 
        width, height,
        ColliderLayer::Blocks, 
        true
    );
}