#include "./UIAnimation.h"

UIAnimation::UIAnimation(
    Game *game,
    const std::string &animPath, 
    const std::string &animData,
    const Vector2 &pos, 
    const Vector2 &size,
    float animFPS,
    int animationStartIdx, int animationEndIdx, bool isLoop
) : UIElement(pos, size, Color::White),
    mGame(game),
    mSpriteSheetTexture(nullptr), 
    mAnimTimer(0.0f), 
    mAnimFPS(animFPS), 
    mIsPaused(false), 
    mAnimation(nullptr)
{
    LoadSpriteSheet(animPath, animData);

    std::vector<int> spriteNums;
    
    for (int i = animationStartIdx; i <= animationEndIdx; ++i) {
        spriteNums.emplace_back(i);
    }

    mAnimation = new Animation(spriteNums, isLoop);
}

void UIAnimation::Update(float deltaTime)
{
    if (mIsPaused) {
        return;
    }

    mAnimTimer += mAnimFPS * deltaTime;

    if (mAnimTimer >= mAnimation->frames.size()) {
        if (mAnimation->isLoop) {
            mAnimTimer = 0.0f;
        }

        else {
            // block in last frame
            mAnimTimer = static_cast<float>(mAnimation->frames.size() - 1);
        }
    }
}

void UIAnimation::Draw(SDL_Renderer* renderer, const Vector2 &screenPos, const Vector3 &modColor)
{
    int spriteIdx = mAnimation->frames[static_cast<int>(mAnimTimer)];
    SDL_Rect *srcRect = mSpriteSheetData[spriteIdx];

    SDL_Rect dstRect = {
        static_cast<int>(screenPos.x + GetPosition().x),
        static_cast<int>(screenPos.y + GetPosition().y),
        srcRect->w,
        srcRect->h};

    SDL_SetTextureBlendMode(mSpriteSheetTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mSpriteSheetTexture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    SDL_RenderCopyEx(renderer, mSpriteSheetTexture, srcRect, &dstRect, 
                     0, nullptr, SDL_FLIP_NONE);
}

void UIAnimation::LoadSpriteSheet(const std::string &texturePath, const std::string &dataPath)
{
    // Load sprite sheet texture
    mSpriteSheetTexture = mGame->LoadTexture(texturePath);

    // Load sprite sheet data
    std::ifstream spriteSheetFile(dataPath);
    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);

    std::vector<std::pair<std::string, SDL_Rect*>> rects;
    rects.reserve(spriteSheetData["frames"].size());

    for (const auto &frame : spriteSheetData["frames"])
    {
        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();
        std::string fileName = frame["filename"].get<std::string>();

        SDL_Rect *r = new SDL_Rect{ x, y, w, h };
        rects.emplace_back(fileName, r);
    }

    // Move sorted rect pointers into mSpriteSheetData
    mSpriteSheetData.reserve(rects.size());
    for (auto &p : rects)
    {
        mSpriteSheetData.emplace_back(p.second);
    }
}