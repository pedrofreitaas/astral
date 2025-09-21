#pragma once

#include <string>
#include <SDL_image.h>
#include "UIImage.h"
#include "../libs/Math.h"

class UICursor : public UIImage
{
public:
    UICursor(const std::string &imagePath,
            const Vector2 &initialMousePos = Vector2::Zero,
            const Vector2 &size = Vector2(100.f, 100.f), 
            const Vector3 &color = Color::White);

    ~UICursor();

    void Draw(SDL_Renderer* renderer, const Vector2 &mousePos) override;
};
