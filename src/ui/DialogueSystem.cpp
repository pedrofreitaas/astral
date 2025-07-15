#include "DialogueSystem.h"
#include "../core/Game.h"
#include "UIFont.h"

DialogueSystem* DialogueSystem::sInstance = nullptr;

DialogueSystem* DialogueSystem::Get()
{
    if (sInstance == nullptr)
    {
        sInstance = new DialogueSystem();
    }
    return sInstance;
}

DialogueSystem::DialogueSystem()
    : mGame(nullptr), mFont(nullptr), mSmallFont(nullptr), mCurrentLine(0), mTextTexture(nullptr),
      mIsActive(false), mContinuePressed(false), mPromptTexture(nullptr)
{
}

DialogueSystem::~DialogueSystem()
{
    if (mFont)
    {
        TTF_CloseFont(mFont);
    }
    if (mSmallFont)
    {
        TTF_CloseFont(mSmallFont);
    }
    if (mTextTexture)
    {
        SDL_DestroyTexture(mTextTexture);
    }
    if (mPromptTexture)
    {
        SDL_DestroyTexture(mPromptTexture);
    }
}

void DialogueSystem::Initialize(Game* game)
{
    mGame = game;
    mFont = TTF_OpenFont("../assets/Fonts/SMB.ttf", 16);
    if (!mFont)
    {
        SDL_Log("Falha ao carregar a fonte para DialogueSystem: %s", TTF_GetError());
    }

    mSmallFont = TTF_OpenFont("../assets/Fonts/SMB.ttf", 10);
    if (!mSmallFont)
    {
        SDL_Log("Falha ao carregar a fonte pequena para DialogueSystem: %s", TTF_GetError());
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize(mGame->GetWindow(), &windowWidth, &windowHeight);
    mBoxRect = {50, windowHeight - 130, windowWidth - 100, 100};

    if (mSmallFont)
    {
        SDL_Color grey = {180, 180, 180, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(mSmallFont, "Pressione Enter", grey);
        if (surface)
        {
            mPromptTexture = SDL_CreateTextureFromSurface(mGame->GetRenderer(), surface);
            mPromptRect.w = surface->w;
            mPromptRect.h = surface->h;
            SDL_FreeSurface(surface);
        }
    }
}

void DialogueSystem::StartDialogue(const std::vector<std::string>& lines, std::function<void()> onComplete)
{
    if (mIsActive) return;

    mLines = lines;
    mOnComplete = onComplete;
    mCurrentLine = 0;
    mIsActive = true;
    mContinuePressed = true;

    mGame->SetGamePlayState(Game::GamePlayState::Dialogue);

    CreateTextTexture();
    mPromptRect.x = mBoxRect.x + mBoxRect.w - mPromptRect.w - 20;
    mPromptRect.y = mBoxRect.y + mBoxRect.h - mPromptRect.h - 15;
}

void DialogueSystem::HandleInput(const uint8_t* keyState)
{
    if (!mIsActive) return;

    if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER])
    {
        if (!mContinuePressed)
        {
            mContinuePressed = true;
            mCurrentLine++;

            if (mCurrentLine >= mLines.size())
            {
                mIsActive = false;
                
                if (mOnComplete)
                {
                    mOnComplete();
                }
            }
            else
            {
                CreateTextTexture();
            }
        }
    }
    else
    {
        mContinuePressed = false;
    }
}

void DialogueSystem::Draw(SDL_Renderer* renderer)
{
    if (!mIsActive) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &mBoxRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &mBoxRect);

    if (mTextTexture)
    {
        SDL_RenderCopy(renderer, mTextTexture, nullptr, &mTextRect);
    }
    if (mPromptTexture)
    {
        SDL_RenderCopy(renderer, mPromptTexture, nullptr, &mPromptRect);
    }
}

void DialogueSystem::CreateTextTexture()
{
    if (mTextTexture)
    {
        SDL_DestroyTexture(mTextTexture);
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(mFont, mLines[mCurrentLine].c_str(), white, mBoxRect.w - 40);
    if (surface)
    {
        mTextTexture = SDL_CreateTextureFromSurface(mGame->GetRenderer(), surface);
        mTextRect = {mBoxRect.x + 20, mBoxRect.y + 20, surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
}