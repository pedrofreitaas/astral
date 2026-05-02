#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include <unordered_map>
#include <functional>
#include <utility>
#include <string>
#include <SDL_image.h>
#include "UIElement.h"
#include "../libs/Math.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../core/Game.h"

class UIAnimation : public UIElement
{
public:
    UIAnimation(
        Game *game,
        const std::string &animPath, 
        const std::string &animData,
        const Vector2 &pos = Vector2::Zero,
        const Vector2 &size = Vector2(100.f, 100.f),
        float animFPS = 10.0f,
        int animationStartIdx=0, int animationEndIdx=0, bool isLoop=true
    );

    ~UIAnimation() = default;

    void Update(float deltaTime);
    void Draw(SDL_Renderer* renderer, const Vector2 &screenPos, const Vector3 &modColor);

private:
    SDL_Texture* mSpriteSheetTexture;
    std::vector<SDL_Rect*> mSpriteSheetData;
    Animation *mAnimation;
    float mAnimTimer;
    float mAnimFPS;
    bool mIsPaused;
    Game *mGame;

    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);
};

#endif // UI_ANIMATION_H