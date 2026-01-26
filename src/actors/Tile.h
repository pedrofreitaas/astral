#pragma once

#include "Actor.h"
#include <string>
#include "../components/draw/DrawComponent.h"
#include "./Snow.h"

enum class SnowDirection;

const int FREEZING_RATE = 1.f;

class Tile : public Actor
{
private:
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnUpdate(float deltaTime) override;

    float mFreezingCount;
    bool mIsFrozen;

    SnowDirection mLastSnowCollision;

    void Freeze();
    void StopFreeze();

    class Snow *mSnow;

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