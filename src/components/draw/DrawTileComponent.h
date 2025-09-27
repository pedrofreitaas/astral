#pragma once

#include <SDL.h>
#include <string>
#include "DrawComponent.h"

class DrawTileComponent : public DrawComponent
{
public:
    DrawTileComponent(
        class Actor* owner, 
        SDL_Texture* tilesetTexture,
        const Vector2& tilesetPosition,
        int width, int height,
        int drawOrder = 100
    );
    ~DrawTileComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;

protected:
    // Map of textures loaded
    SDL_Texture* mTilesetSurface;
    Vector2 mTilesetPosition;
    int mWidth, mHeight;
};