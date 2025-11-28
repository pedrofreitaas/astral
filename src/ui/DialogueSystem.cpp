#include "DialogueSystem.h"
#include "../core/Game.h"
#include "UIFont.h"
#include "../actors/Zoe.h"

DialogueSystem::DialogueSystem(Game::GamePlayState currentGameState)
    : mGame(nullptr), mFont(nullptr), mSmallFont(nullptr), mCurrentLine(0), mTextTexture(nullptr),
      mIsActive(false), mContinuePressed(false), mPromptTexture(nullptr), mPreviousGameState(currentGameState),
      mSpeakerName(""), mSpeakerTexture(nullptr), mSpeakerOffset(-5.0f, -10.0f), mTextOffset(0.0f, 0.0f),
      mLineTimer(0.0f), mLineDuration(5.0f), mStepDialogueSound(SoundHandle::Invalid)
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
    if (mSpeakerTexture)
    {
        SDL_DestroyTexture(mSpeakerTexture);
    }
}

void DialogueSystem::Initialize(Game* game)
{
    mGame = game;
    mFont = TTF_OpenFont("../assets/Fonts/SMB.ttf", 24);
    if (!mFont)
    {
        SDL_Log("Falha ao carregar a fonte para DialogueSystem: %s", TTF_GetError());
    }

    mSmallFont = TTF_OpenFont("../assets/Fonts/SMB.ttf", 16);
    if (!mSmallFont)
    {
        SDL_Log("Falha ao carregar a fonte pequena para DialogueSystem: %s", TTF_GetError());
    }

    int windowWidth = mGame->GetRealWindowWidth();
    int windowHeight = mGame->GetRealWindowHeight();
    int mWidth = static_cast<int>(windowWidth * 3/4);
    int mHeight = static_cast<int>(windowWidth * 1/6);
    mBoxRect = {windowWidth / 2 - mWidth / 2, windowHeight - mHeight - 10, mWidth, mHeight};

    if (mSmallFont)
    {
        SDL_Color grey = {180, 180, 180, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(mSmallFont, "Pressione B", grey);
        if (surface)
        {
            mPromptTexture = SDL_CreateTextureFromSurface(mGame->GetRenderer(), surface);
            mPromptRect.w = surface->w;
            mPromptRect.h = surface->h;
            SDL_FreeSurface(surface);
        }
    }

    // Create initial speaker texture (empty)
    CreateSpeakerTexture();
}

void DialogueSystem::StartDialogue(const std::vector<std::string>& lines, std::function<void()> onComplete)
{
    if (mIsActive) return;

    mLines = lines;
    mOnComplete = onComplete;
    mCurrentLine = 0;
    mIsActive = true;
    mContinuePressed = true;
    mLineTimer = 0.0f;

    mGame->SetGamePlayState(Game::GamePlayState::Dialogue);

    CreateTextTexture();
    mPromptRect.x = mBoxRect.x + mBoxRect.w - mPromptRect.w - 20;
    mPromptRect.y = mBoxRect.y + mBoxRect.h - mPromptRect.h - 15;
    
    // Position speaker box in upper-left corner of dialogue box with offset
    mSpeakerRect.x = mBoxRect.x + 10 + static_cast<int>(mSpeakerOffset.x);
    mSpeakerRect.y = mBoxRect.y - mSpeakerRect.h / 2 + static_cast<int>(mSpeakerOffset.y);
}

void DialogueSystem::StartDialogueWithSpeaker(const std::string& speaker, const std::vector<std::string>& lines, std::function<void()> onComplete)
{
    // Set the speaker name
    SetSpeakerName(speaker);
    
    // Start the dialogue normally
    StartDialogue(lines, [this, onComplete]() {
        // Reset speaker to empty when dialogue completes
        SetSpeakerName("");
        // Call the original onComplete callback
        if (onComplete)
        {
            onComplete();
        }
    });
}

void DialogueSystem::HandleInput(const uint8_t* keyState)
{
    if (!mIsActive) return;

    SDL_GameController *controller = mGame->GetController();
    const bool hasController = controller && SDL_GameControllerGetAttached(controller);

    if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_KP_ENTER] ||
        (hasController && SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)))
    {
        if (!mContinuePressed)
        {
            mContinuePressed = true;
            AdvanceDialogue();
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

    // Draw the timer progress bar
    if (mLineDuration > 0.0f)
    {
        float progress = 1.0f - (mLineTimer / mLineDuration);
        int barWidth = static_cast<int>((mBoxRect.w - 40) * progress);
        if (barWidth > 0)
        {
            SDL_Rect progressBar = {
                mBoxRect.x + 20,
                mBoxRect.y - 10, // Position above the dialogue box
                barWidth,
                5
            };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &progressBar);
        }
    }

    // Draw speaker box only if speaker name is not empty
    if (mSpeakerTexture && !mSpeakerName.empty())
    {
        // Draw background for speaker
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect speakerBgRect = {mSpeakerRect.x - 5, mSpeakerRect.y - 5, 
                                  mSpeakerRect.w + 10, mSpeakerRect.h + 10};
        SDL_RenderFillRect(renderer, &speakerBgRect);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &speakerBgRect);
        
        SDL_RenderCopy(renderer, mSpeakerTexture, nullptr, &mSpeakerRect);
    }

    if (mTextTexture)
    {
        SDL_RenderCopy(renderer, mTextTexture, nullptr, &mTextRect);
    }
    
    // Move prompt above the dialogue box
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
        mTextRect = {mBoxRect.x + 20 + static_cast<int>(mTextOffset.x), 
                     mBoxRect.y + 20 + static_cast<int>(mTextOffset.y), 
                     surface->w, surface->h};
        SDL_FreeSurface(surface);
    }
}

void DialogueSystem::CreateSpeakerTexture()
{
    if (mSpeakerTexture)
    {
        SDL_DestroyTexture(mSpeakerTexture);
        mSpeakerTexture = nullptr;
    }

    // Only create texture if speaker name is not empty
    if (!mSpeakerName.empty())
    {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(mFont, mSpeakerName.c_str(), white);
        if (surface)
        {
            mSpeakerTexture = SDL_CreateTextureFromSurface(mGame->GetRenderer(), surface);
            mSpeakerRect.w = surface->w;
            mSpeakerRect.h = surface->h;
            SDL_FreeSurface(surface);
        }
    }
}

void DialogueSystem::SetSpeakerName(const std::string& name)
{
    if (mSpeakerName != name)
    {
        mSpeakerName = name;
        CreateSpeakerTexture();
        
        // Reposition the speaker box if the dialogue is active and name is not empty
        if (!mIsActive || mSpeakerName.empty())
        {
            return;
        }
        
        mSpeakerRect.x = mBoxRect.x + 10 + static_cast<int>(mSpeakerOffset.x);
        mSpeakerRect.y = mBoxRect.y - mSpeakerRect.h / 2 + static_cast<int>(mSpeakerOffset.y);
    }
}

void DialogueSystem::Update(float deltaTime)
{
    if (!mIsActive) return;

    // Update the timer
    mLineTimer += deltaTime;
    
    // Check if the timer has reached the line duration
    if (mLineTimer >= mLineDuration)
    {
        AdvanceDialogue();
    }
}

void DialogueSystem::AdvanceDialogue()
{
    mLineTimer = 0.0f;
    mCurrentLine++;
    
    if (!mStepDialogueSound.IsValid() || mGame->GetAudio()->GetSoundState(mStepDialogueSound) != SoundState::Playing)
    {
        mStepDialogueSound = mGame->GetAudio()->PlaySound("dialogueStep.wav");
    }

    if (mCurrentLine >= mLines.size())
    {
        mIsActive = false;
        
        if (mOnComplete)
        {
            mGame->GoBackToPreviousGameState();
            mOnComplete();
        }
    }
    else
    {
        CreateTextTexture();
    }
}
