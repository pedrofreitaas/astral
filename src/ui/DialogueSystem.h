#pragma once
#include <vector>
#include <string>
#include <functional>
#include <SDL_ttf.h>
#include "../core/Game.h"
#include "../libs/Math.h"

class Game;
struct SDL_Renderer;

class DialogueSystem
{
public:
    DialogueSystem(Game::GamePlayState currentGameState);
    ~DialogueSystem();
    static DialogueSystem* Get();
    void Initialize(Game* game);
    void StartDialogue(const std::vector<std::string>& lines, std::function<void()> onComplete);
    void StartDialogueWithSpeaker(const std::string& speaker, const std::vector<std::string>& lines, std::function<void()> onComplete);
    void HandleInput(const uint8_t* keyState);
    void Draw(SDL_Renderer* renderer);
    void Update(float deltaTime);
    bool IsActive() const { return mIsActive; }
    void SetLineDuration(float duration) { mLineDuration = duration; }
    float GetLineDuration() const { return mLineDuration; }

private:
    void CreateTextTexture();
    void CreateSpeakerTexture();
    void SetSpeakerName(const std::string& name);
    void SetSpeakerOffset(const Vector2& offset) { mSpeakerOffset = offset; }
    void SetTextOffset(const Vector2& offset) { mTextOffset = offset; }
    void AdvanceDialogue();
    
    Game* mGame;
    TTF_Font* mFont;
    TTF_Font* mSmallFont;

    std::vector<std::string> mLines;
    size_t mCurrentLine;
    std::function<void()> mOnComplete;

    SDL_Texture* mTextTexture;
    SDL_Rect mBoxRect;
    SDL_Rect mTextRect;

    SDL_Texture* mPromptTexture;
    SDL_Rect mPromptRect;

    SDL_Texture* mSpeakerTexture;
    SDL_Rect mSpeakerRect;
    std::string mSpeakerName;
    Vector2 mSpeakerOffset;
    Vector2 mTextOffset;

    Game::GamePlayState mPreviousGameState;

    bool mIsActive;
    bool mContinuePressed;
    float mLineTimer;
    float mLineDuration;
};
