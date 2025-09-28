//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "DrawSpriteComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"

DrawSpriteComponent::DrawSpriteComponent(class Actor* owner, const std::string &texturePath, const int width, const int height, const int drawOrder)
        :DrawComponent(owner, drawOrder)
        ,mWidth(width)
        ,mHeight(height)
        ,mFlip(false)
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

    SDL_Rect dstrect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
        mWidth,
        mHeight
    };

    SDL_Point center;
    center.x = static_cast<int>(mPivot.x * mWidth);
    center.y = static_cast<int>(mPivot.y * mHeight);
    double rotationDeg = mOwner->GetRotation() * 180.0 / Math::Pi;

    SDL_RendererFlip flip = mFlip ? SDL_FLIP_VERTICAL  : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, mSpriteSheetSurface, nullptr, &dstrect, rotationDeg, &center, flip);
}
