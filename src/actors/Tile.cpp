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
    if (other->GetLayer() == ColliderLayer::Nevasca && !IsFrozen())
    {
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::RIGHT :SnowDirection::LEFT;
    }
}

void Tile::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Nevasca && !IsFrozen())
    {
        mLastSnowCollision = minOverlap > 0 ? SnowDirection::DOWN : SnowDirection::UP;
    }
}

void Tile::OnUpdate(float deltatime)
{
    Actor::OnUpdate(deltatime);
}

void Tile::Freeze()
{
    if (IsFrozen()) return;

    if (mSnow != nullptr) SDL_Log("Warning: freezing a object that has snow in memory.");
    
    mSnow = new Snow(
        mGame,
        GetCenter(),
        mLastSnowCollision);

    mBehaviorState = BehaviorState::Frozen;
}

void Tile::StopFreeze()
{
    if (!IsFrozen()) return;

    if (mSnow != nullptr) mSnow->Kill();
    mSnow = nullptr;

    mBehaviorState = BehaviorState::Idle;
}