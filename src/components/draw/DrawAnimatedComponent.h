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
    void AddAnimation(const std::string& name, int begin, int end, bool isLoop=true);

    void Scale(int scale) {
        mScaleFactor = scale;
    };

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

    void LoadSpriteSheet(const std::string& texturePath, const std::string& dataPath);
};