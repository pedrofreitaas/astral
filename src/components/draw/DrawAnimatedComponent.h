//
// Created by Lucas N. Ferreira on 28/09/23.
//
#pragma once
#include <unordered_map>
#include <functional>
#include <utility>
#include "DrawComponent.h"

class Animation {
public:
    std::vector<int> frames;
    bool isLoop;

    Animation(std::vector<int> frames, bool isLoop): frames(std::move(frames)), isLoop(isLoop) {}
    Animation() : frames(), isLoop(true) {}
};

class DrawAnimatedComponent : public DrawComponent {
public:
    // (Lower draw order corresponds with further back)
    DrawAnimatedComponent(
        class Actor* owner, 
        const std::string &spriteSheetPath, 
        const std::string &spriteSheetData,
        std::function<void(std::string animationName)> animationEndCallback = nullptr,
        int drawOrder = 100);
    ~DrawAnimatedComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;
    void Update(float deltaTime) override;

    // Use to change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    // Set the current active animation
    void SetAnimation(const std::string& name);

    // Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    // Add an animation of the corresponding name to the animation map
    void AddAnimation(const std::string& name, const std::vector<int>& images, bool isLoop=true);
    
    // Add an animation of the corresponding name to the animation map
    // - end is inclusive
    void AddAnimation(const std::string& name, int begin, int end, bool isLoop=true);

    void Scale(int scale) {
        mScaleFactor = scale;
    };

    void SetPivot(const Vector2& pivot) { mPivot = pivot; }
    const Vector2& GetPivot() const { return mPivot; }

    void SetUsePivotForRotation(bool usePivot) { mUsePivotForRotation = usePivot; }
    bool GetUsePivotForRotation() const { return mUsePivotForRotation; }

    int GetSpriteWidth() const {
        if (mSpriteSheetData.empty()) return 0;
        return mSpriteSheetData[0]->w * mScaleFactor;
    };

    int GetSpriteHeight() const {
        if (mSpriteSheetData.empty()) return 0;
        return mSpriteSheetData[0]->h * mScaleFactor;
    };

    Vector2 GetSpriteSize() const {
        if (mSpriteSheetData.empty()) return Vector2::Zero;
        return Vector2(
            static_cast<float>(mSpriteSheetData[0]->w * mScaleFactor),
            static_cast<float>(mSpriteSheetData[0]->h * mScaleFactor)
        );
    };

    Vector2 GetHalfSpriteSize() const {
        if (mSpriteSheetData.empty()) return Vector2::Zero;
        return Vector2(
            static_cast<float>(mSpriteSheetData[0]->w * 0.5f * mScaleFactor),
            static_cast<float>(mSpriteSheetData[0]->h * 0.5f * mScaleFactor)
        );
    };

    // ZERO INDEXED
    int GetCurrentSprite() {
        if (mAnimName.empty()) return -1;
        int firstFrame = mAnimations[mAnimName].frames[0];
        return mAnimations[mAnimName].frames[static_cast<int>(mAnimTimer)] - firstFrame;
    }

private:
    std::vector<SDL_Rect*> mSpriteSheetData;
    std::unordered_map<std::string, class Animation> mAnimations;
    std::function<void(std::string animationName)> mAnimationEndCallback;
    std::string mAnimName;
    SDL_Texture* mSpriteSheetTexture;
    float mAnimTimer;
    float mAnimFPS;
    bool mIsPaused;
    int mScaleFactor;
    Vector2 mPivot;
    bool mUsePivotForRotation;

    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);
};
