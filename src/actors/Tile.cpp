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
    const DrawLayerPosition &layer) : Actor(game), mSnow(nullptr), mLastSnowCollision(SnowDirection::UP)
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
        static_cast<int>(layer));

    bool hasCollision = (layer == DrawLayerPosition::Player);

    if (!hasCollision)
        return;

    new AABBColliderComponent(
        this,
        boundBoxOffsetX, boundBoxOffsetY,
        boundBoxWidth, boundBoxHeight,
        ColliderLayer::Blocks);
}

void Tile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Nevasca && !mIsFrozen)
    {
        // gravity causes more VERTICAL collisions, so this is to keep a similar sensation of freezing,
        // so we need a bigger increment when colliding horizontally
        IncreaseFreezing(10.f);
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::RIGHT :SnowDirection::LEFT;
    }
}

void Tile::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Nevasca && !mIsFrozen)
    {
        IncreaseFreezing();
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::DOWN : SnowDirection::UP;
    }
}

void Tile::OnUpdate(float deltatime)
{
    Actor::OnUpdate(deltatime);
}

void Tile::Freeze()
{
    if (mIsFrozen) return;

    if (mSnow != nullptr) SDL_Log("Warning: freezing a object that has snow in memory.");
    
    mSnow = new Snow(
        mGame,
        GetCenter(),
        mLastSnowCollision);

    mIsFrozen = true;
}

void Tile::StopFreeze()
{
    if (!mIsFrozen) return;

    mSnow->Kill();
    mSnow = nullptr;

    mIsFrozen = false;
}