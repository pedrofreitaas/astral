//
// Created by Lucas N. Ferreira on 03/08/23.
//

#pragma once
#include "../Component.h"
#include "../../libs/Math.h"
#include <vector>
#include <SDL.h>

enum class DrawLayerPosition
{
    Ground = 0,
    Player = 100,
    Enemy = 101,
    DetailsTop = 150,
    DetailsDown = 50,
    Sky = 200
};

constexpr DrawLayerPosition Layers[] = {
  DrawLayerPosition::Ground,
  DrawLayerPosition::Player,
  DrawLayerPosition::DetailsTop,
  DrawLayerPosition::DetailsDown,
  DrawLayerPosition::Sky
};

class DrawComponent : public Component
{
public:
    // (Lower draw order corresponds with further back)
    explicit DrawComponent(class Actor* owner, int drawOrder = 100);
    ~DrawComponent() override;

    virtual void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White);

    bool IsVisible() const { return mIsVisible; }
    void SetIsVisible(const bool isVisible) { mIsVisible = isVisible; }

    int GetDrawOrder() const { return mDrawOrder; }

protected:
    bool mIsVisible;
    int mDrawOrder;
};
