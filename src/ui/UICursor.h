#pragma once

#include <string>
#include <SDL_image.h>
#include "UIImage.h"
#include "../libs/Math.h"
#include "../core/Game.h"

class UICursor : public UIImage
{
public:
    UICursor(class Game *game,
            const std::string &imagePath,
            const Vector2 &initialPos = Vector2::Zero,
            const Vector2 &size = Vector2(100.f, 100.f), 
            const Vector3 &color = Color::White);

    void Update(float deltaTime);
    ~UICursor();

private:
    float mFowardSpeed;
    Vector2 mSpeed;
    Game* mGame;
};
