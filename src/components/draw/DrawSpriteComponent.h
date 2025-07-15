//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "DrawComponent.h"
#include <string>

class DrawSpriteComponent : public DrawComponent
{
public:
    // (Lower draw order corresponds with further back)
    DrawSpriteComponent(class Actor* owner, const std::string &texturePath, int width = 0, int height = 0, int drawOrder = 100);
    ~DrawSpriteComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;

    void SetFlip(bool flip) { mFlip = flip; }
    void SetPivot(const Vector2& pivot) { mPivot = pivot; }

protected:
    // Map of textures loaded
    SDL_Texture* mSpriteSheetSurface;

    int mWidth;
    int mHeight;
    bool mFlip = false;
    Vector2 mPivot{0.5f, 0.5f};
};