#pragma once

#include <SDL.h>
#include <string>
#include "DrawComponent.h"

class DrawTileComponent : public DrawComponent
{
public:
    // (Lower draw order corresponds with further back)
    DrawTileComponent(
        class Actor* owner, 
        SDL_Texture* tilesetTexture,
        int gridX, int gridY,
        int width, int height,
        int drawOrder = 100
    );
    ~DrawTileComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;

protected:
    // Map of textures loaded
    SDL_Texture* mTilesetSurface;

    int mGridX, mGridY;
    int mWidth;
    int mHeight;
};