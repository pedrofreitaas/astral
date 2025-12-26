//
// Created by Lucas N. Ferreira on 22/05/25.
//

#pragma once

#include "../libs/Math.h"
#include <SDL.h>

class UIElement {
public:
    UIElement(const Vector2 &pos, const Vector2 &size, const Vector3 &color);

    const Vector2& GetPosition() const;
    const Vector2& GetCenter() const;
    void SetPosition(const Vector2 &pos);

    const Vector2& GetSize() const { return mSize; }
    void SetSize(const Vector2 &size) { mSize = size; }

    const Vector3& GetColor() const { return mColor; }
    void SetColor(const Vector3 &color) { mColor = color; }

    virtual void Draw(SDL_Renderer* renderer, const Vector2 &screenPos) {};
    bool IsEnabled() const { return mIsEnabled; }
    void SetEnabled(bool enabled) { mIsEnabled = enabled; }

protected:
    Vector2 mPosition;
    Vector2 mSize;
    Vector3 mColor;
    bool mIsEnabled;
};
