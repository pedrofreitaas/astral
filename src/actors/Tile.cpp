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
    const DrawLayerPosition &layer) : Actor(game), mFreezingCount(0), mIsFrozen(false),
                                      mSnow(nullptr), mLastSnowCollision(SnowDirection::UP)
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
        mFreezingCount += mGame->GetDtLastFrame() * FREEZING_RATE * 10.f; // gravity causes more VERTICAL collisions, so this is to keep a similar sensation of freezing
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::RIGHT :SnowDirection::LEFT;
    }
}

void Tile::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Nevasca && !mIsFrozen)
    {
        mFreezingCount += mGame->GetDtLastFrame() * FREEZING_RATE;
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::DOWN : SnowDirection::UP;
    }
}

void Tile::OnUpdate(float deltatime)
{
    if (mIsFrozen && mFreezingCount > 0.f)
    {
        mFreezingCount -= deltatime * (FREEZING_RATE * .33f);
        return;
    }

    mFreezingCount -= deltatime * (FREEZING_RATE * .5f);

    if (mFreezingCount <= 0.f)
    {
        mFreezingCount = 0.f;
        StopFreeze();
        return;
    }
    
    if (mFreezingCount >= 1.f)
    {
        mFreezingCount = 1.f;
        Freeze();
        return;
    }
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

    mSnow->Unspawn();
    mSnow = nullptr;

    mIsFrozen = false;
}