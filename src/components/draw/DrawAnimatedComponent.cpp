
#include "DrawAnimatedComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"
#include "../../libs/Json.h"
#include <fstream>

DrawAnimatedComponent::DrawAnimatedComponent(
    class Actor *owner, 
    const std::string &spriteSheetPath, 
    const std::string &spriteSheetData, 
    std::function<void(std::string animationName)> animationEndCallback,
    int drawOrder): 
    DrawComponent(owner, drawOrder), 
    mSpriteSheetTexture(nullptr), mAnimTimer(0.0f), mAnimFPS(10.0f), 
    mIsPaused(false), mAnimName(""), mAnimationEndCallback(animationEndCallback),
    mScaleFactor(1.0f), mPivot(0.5f, 0.5f), mUsePivotForRotation(false)
{
    LoadSpriteSheet(spriteSheetPath, spriteSheetData);
}

DrawAnimatedComponent::~DrawAnimatedComponent()
{
    DrawComponent::~DrawComponent();

    for (const auto &rect : mSpriteSheetData)
    {
        delete rect;
    }
    mSpriteSheetData.clear();
}

void DrawAnimatedComponent::LoadSpriteSheet(const std::string &texturePath, const std::string &dataPath)
{
    // Load sprite sheet texture
    mSpriteSheetTexture = mOwner->GetGame()->LoadTexture(texturePath);

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

    // Sort by filename (lexicographically)
    std::sort(rects.begin(), rects.end(),
              [](const std::pair<std::string, SDL_Rect*> &a,
                 const std::pair<std::string, SDL_Rect*> &b) {
                  return a.first < b.first;
              });

    // Move sorted rect pointers into mSpriteSheetData
    mSpriteSheetData.reserve(rects.size());
    for (auto &p : rects)
    {
        mSpriteSheetData.emplace_back(p.second);
    }
}

void DrawAnimatedComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor)
{
    int spriteIdx = mAnimations[mAnimName].frames[static_cast<int>(mAnimTimer)];
    SDL_Rect *srcRect = mSpriteSheetData[spriteIdx];

    SDL_Rect dstRect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
        srcRect->w * mScaleFactor,
        srcRect->h * mScaleFactor};

    // Calculate pivot point
    SDL_Point pivotPoint;
    pivotPoint.x = static_cast<int>(mPivot.x * dstRect.w);
    pivotPoint.y = static_cast<int>(mPivot.y * dstRect.h);

    SDL_SetTextureBlendMode(mSpriteSheetTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mSpriteSheetTexture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    // Use pivot point for rotation only if enabled
    if (mUsePivotForRotation)
    {
        SDL_RenderCopyEx(renderer, mSpriteSheetTexture, srcRect, &dstRect, 
                     Math::ToDegrees(mOwner->GetRotation()), &pivotPoint, SDL_FLIP_NONE);
        return;
    }
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (mOwner->GetRotation() == Math::Pi)
    {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_RenderCopyEx(renderer, mSpriteSheetTexture, srcRect, &dstRect, 
                     mOwner->GetRotation(), nullptr, flip);
}

void DrawAnimatedComponent::Update(float deltaTime)
{
    if (mIsPaused) {
        return;
    }

    mAnimTimer += mAnimFPS * deltaTime;

    if (mAnimTimer >= mAnimations[mAnimName].frames.size()) {
        if (mAnimationEndCallback) mAnimationEndCallback(mAnimName);

        if (mAnimations[mAnimName].isLoop) {
            mAnimTimer = 0.0f;
        }
        else {
            // block in last frame
            mAnimTimer = static_cast<float>(mAnimations[mAnimName].frames.size() - 1);
        }
    }
}

void DrawAnimatedComponent::SetAnimation(const std::string &name)
{
    if (mAnimName == name) {
        return;
    }

    mAnimTimer = 0.0f;
    mAnimName = name;
    Update(0.0f);
}

void DrawAnimatedComponent::AddAnimation(const std::string &name, const std::vector<int> &spriteNums, bool isLoop)
{
    mAnimations.emplace(name, std::move(Animation(spriteNums, isLoop)));
}

void DrawAnimatedComponent::AddAnimation(const std::string &name, int begin, int end, bool isLoop)
{
    std::vector<int> spriteNums;
    for (int i = begin; i <= end; ++i) {
        spriteNums.emplace_back(i);
    }
    mAnimations.emplace(name, std::move(Animation(spriteNums, isLoop)));
}

