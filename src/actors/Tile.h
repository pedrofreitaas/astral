#pragma once

#include "Actor.h"
#include <string>
#include "../components/draw/DrawComponent.h"
#include "./Snow.h"

enum class SnowDirection;

class Tile : public Actor
{
private:
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnUpdate(float deltaTime) override;
    void StopFreeze() override;
    void Freeze() override;
    
    class Snow *mSnow;
    SnowDirection mLastSnowCollision;

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