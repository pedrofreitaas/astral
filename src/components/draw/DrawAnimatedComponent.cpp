
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
    mIsPaused(false), mAnimName(""), mAnimationEndCallback(animationEndCallback)
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

    SDL_Rect *rect = nullptr;
    for (const auto &frame : spriteSheetData["frames"])
    {
        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();
        rect = new SDL_Rect({x, y, w, h});

        mSpriteSheetData.emplace_back(rect);
    }
}

void DrawAnimatedComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor)
{
    int spriteIdx = mAnimations[mAnimName].frames[static_cast<int>(mAnimTimer)];
    SDL_Rect *srcRect = mSpriteSheetData[spriteIdx];

    SDL_Rect dstRect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
        srcRect->w,
        srcRect->h};

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (mOwner->GetRotation() == Math::Pi)
    {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_SetTextureBlendMode(mSpriteSheetTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mSpriteSheetTexture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    SDL_RenderCopyEx(renderer, mSpriteSheetTexture, srcRect, &dstRect, mOwner->GetRotation(), nullptr, flip);
}

void DrawAnimatedComponent::Update(float deltaTime)
{
    if (mIsPaused) {
        return;
    }

    mAnimTimer += mAnimFPS * deltaTime;

    if (mAnimTimer >= mAnimations[mAnimName].frames.size()) {
        if (mAnimationEndCallback) mAnimationEndCallback(mAnimName);

        do {
            if (mAnimations[mAnimName].isLoop) {
                mAnimTimer -= mAnimations[mAnimName].frames.size();
                continue;
            }

            mAnimTimer = static_cast<float>(mAnimations[mAnimName].frames.size() - 1);
        } while (mAnimTimer >= mAnimations[mAnimName].frames.size());
    }
}

void DrawAnimatedComponent::SetAnimation(const std::string &name)
{
    mAnimName = name;
    Update(0.0f);
}

void DrawAnimatedComponent::AddAnimation(const std::string &name, const std::vector<int> &spriteNums, bool isLoop)
{
    mAnimations.emplace(name, std::move(Animation(spriteNums, isLoop)));
}
