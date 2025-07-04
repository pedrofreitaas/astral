#pragma once
#include "DrawComponent.h"

class DrawRectangleComponent : public DrawComponent
{
public:
    DrawRectangleComponent(class Actor* owner, Vector2 size, Vector3 color, int drawOrder = 100);
    void SetSize(const Vector2& size);
    void SetColor(const Vector3& color);

    void Draw(SDL_Renderer* renderer, const Vector3& modColor) override;

private:
    Vector2 mSize;
    Vector3 mColor;
};