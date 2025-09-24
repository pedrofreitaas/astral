#include "./DrawTileComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"

DrawTileComponent::DrawTileComponent(
    class Actor* owner, 
    SDL_Texture* tilesetTexture,
    int gridX, int gridY,
    int width, int height,
    int drawOrder
)
    : DrawComponent(owner, drawOrder)
    , mWidth(width)
    , mHeight(height)
    , mGridX(gridX)
    , mGridY(gridY)
{
    if (!tilesetTexture) {
        throw std::runtime_error("DrawTileComponent requires a valid tileset texture.");
    }

    mTilesetSurface = tilesetTexture;
}

DrawTileComponent::~DrawTileComponent()
{
    DrawComponent::~DrawComponent();
}

void DrawTileComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor)
{
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    SDL_Rect srcrect = {
        mGridX * mWidth,
        mGridY * mHeight,
        mWidth,
        mHeight
    };

    SDL_Rect dstrect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
        mWidth,
        mHeight
    };

    SDL_RenderCopyEx(renderer, mTilesetSurface, nullptr, &dstrect, 0.0f, nullptr, SDL_FLIP_NONE);
}