#include "Tile.h"
#include "../core/Game.h"
#include "../components/draw/DrawTileComponent.h"
#include "../components/collider/AABBColliderComponent.h"

Tile::Tile(
    Game *game, 
    SDL_Texture *tilesetTexture,
    const Vector2 &worldPosition,
    const Vector2 &tilesetPosition,
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

    mPosition = worldPosition;

    mGame->Reinsert(this);

    new DrawTileComponent(
        this,
        tilesetTexture,
        tilesetPosition,
        width, height,
        static_cast<int>(layer)
    );

    bool hasCollision = (layer == DrawLayerPosition::Player);

    if (!hasCollision)
        return;

    new AABBColliderComponent(
        this, 
        boundBoxOffsetX, boundBoxOffsetY, 
        boundBoxWidth, boundBoxHeight,
        ColliderLayer::Blocks
    );
}