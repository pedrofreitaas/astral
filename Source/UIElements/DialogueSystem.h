#pragma once
#include <vector>
#include <string>
#include <functional>
#include <SDL_ttf.h>

class Game;
struct SDL_Renderer;

class DialogueSystem
{
public:
    static DialogueSystem* Get();
    void Initialize(Game* game);
    void StartDialogue(const std::vector<std::string>& lines, std::function<void()> onComplete);
    void HandleInput(const uint8_t* keyState);
    void Draw(SDL_Renderer* renderer);
    bool IsActive() const { return mIsActive; }

private:
    DialogueSystem();
    ~DialogueSystem();

    void CreateTextTexture();

    static DialogueSystem* sInstance;
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

    bool mIsActive;
    bool mContinuePressed;
};