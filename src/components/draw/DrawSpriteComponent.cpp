//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "DrawSpriteComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"

DrawSpriteComponent::DrawSpriteComponent(
    class Actor* owner, 
    const std::string &texturePath, 
    const int width, const int height, 
    const int drawOrder,
    const Vector2& offset
):  DrawComponent(owner, drawOrder)
    ,mWidth(width)
    ,mHeight(height)
    ,mFlip(false)
    ,mOffset(offset)
{
    mSpriteSheetSurface = mOwner->GetGame()->LoadTexture(texturePath);
}

DrawSpriteComponent::~DrawSpriteComponent()
{
    DrawComponent::~DrawComponent();

    if (mSpriteSheetSurface) {
        SDL_DestroyTexture(mSpriteSheetSurface);
        mSpriteSheetSurface = nullptr;
    }
}

void DrawSpriteComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor)
{
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    SDL_Rect srcrect = {
        mOffset.x, 
        mOffset.y,
        mWidth,
        mHeight
    };

    SDL_Rect dstrect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
        mWidth,
        mHeight
    };

    SDL_Point center;
    center.x = static_cast<int>(mPivot.x * mWidth);
    center.y = static_cast<int>(mPivot.y * mHeight);
    double rotationDeg = Math::ToDegrees(mOwner->GetRotation());

    SDL_RendererFlip flip = mFlip ? SDL_FLIP_VERTICAL  : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &srcrect, &dstrect, rotationDeg, &center, flip);
}
