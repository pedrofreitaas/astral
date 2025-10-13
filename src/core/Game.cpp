// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <filesystem>
#include <regex>
#include "Game.h"
#include "HUD.h"
#include "SpatialHashing.h"
#include "../libs/Json.h"
#include "../libs/Random.h"
#include "../actors/Actor.h"
#include "../actors/Zoe.h"
#include "../actors/Tile.h"
#include "../actors/Item.h"
#include "../ui/UIScreen.h"
#include "../components/draw/DrawComponent.h"
#include "../components/draw/DrawSpriteComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../ui/DialogueSystem.h"
#include "../actors/Star.h"
#include "../actors/Enemy.h"

const int CHAR_WIDTH = 6;
const int WORD_HEIGHT = 8;

Game::Game(int windowWidth, int windowHeight)
    : mWindow(nullptr), mRenderer(nullptr), mTicksCount(0), mIsRunning(true),
      mZoe(nullptr), mHUD(nullptr), mBackgroundColor(0, 0, 0),
      mModColor(255, 255, 255), mCameraPos(Vector2::Zero), mAudio(nullptr),
      mSceneManagerTimer(0.0f), mSceneManagerState(SceneManagerState::None), mGameScene(GameScene::MainMenu),
      mNextScene(GameScene::Level1), mBackgroundTexture(nullptr), mBackgroundSize(Vector2::Zero),
      mBackgroundPosition(Vector2::Zero), mMap(nullptr), mBackgroundIsCameraWise(true),
      mCurrentCutscene(nullptr), mCutscenes(), mGamePlayState(GamePlayState::Playing),
      mDebugMode(false), mPrevDeltaTime(0.0f), mEnemy(nullptr), mStar(nullptr)
{
    mRealWindowWidth = windowWidth;
    mRealWindowHeight = windowHeight;
    mWindowWidth = 640;
    mWindowHeight = 360;

    mDialogueSystem = new DialogueSystem(mGamePlayState);
}

void Game::SetMap(const std::string &path)
{
    mMap = new Map(this, path);
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow(
        "astral",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mRealWindowWidth, mRealWindowHeight,
        SDL_WINDOW_ALWAYS_ON_TOP);

    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    if (SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight) != 0)
    {
        SDL_Log("Failed to set logical size: %s", SDL_GetError());
        return false;
    }

    if (SDL_RenderSetIntegerScale(mRenderer, SDL_FALSE) != 0)
    {
        SDL_Log("Failed to set integer scale: %s", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        SDL_Log("Failed to initialize SDL_mixer");
        return false;
    }

    // Start random number generator
    Random::Init();

    // Initialize game systems
    mAudio = new AudioSystem();
    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f,
                                         LEVEL_WIDTH * TILE_SIZE,
                                         LEVEL_HEIGHT * TILE_SIZE);

    mDialogueSystem->Initialize(this);

    SetGameScene(GameScene::MainMenu);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    if (mSceneManagerState == SceneManagerState::None)
    {
        if (scene == GameScene::MainMenu || scene == GameScene::Level1)
        {
            mNextScene = scene;
            mSceneManagerState = SceneManagerState::Entering;
            mSceneManagerTimer = transitionTime;
            return;
        }

        throw std::runtime_error("Invalid scene");
    }
}

void Game::ResetGameScene(float transitionTime)
{
    SetGameScene(mGameScene, transitionTime);
}

void Game::LoadFirstLevel()
{
    mHUD = new HUD(this, "../assets/Fonts/VT323-Regular.ttf");

    SetMap("demo.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/galaxy.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    mZoe = new Zoe(this, 1500.0f);
    mZoe->SetPosition(Vector2(32.0f, mMap->GetHeight() - 80.0f));
    mEnemy = new Enemy(this, 1500.0f, Vector2(600.0f, mMap->GetHeight() - 80.0f));

    std::vector<std::unique_ptr<Step>> steps;
    steps.push_back(std::make_unique<MoveStep>(this, mZoe, Vector2(96.0f, mZoe->GetPosition().y), 100.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(this, mZoe, Vector2(94.0f, mZoe->GetPosition().y), 20.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    steps.push_back(std::make_unique<MoveStep>(this, mZoe, Vector2(98.0f, mZoe->GetPosition().y), 20.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 1.5f));
    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(0.0f, mMap->GetHeight() - mWindowHeight / 2.0f)));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetStar(); },
        Vector2(mWindowWidth / 2.0f, mMap->GetHeight() - mWindowHeight / 2.0f),
        250.0f));
    std::vector<std::string> dialogue = {
        "Zoe: O que... o que e isso?",
        "Zoe: Uma estrela? Mas como ela foi parar aqui?"};
    steps.push_back(std::make_unique<DialogueStep>(this, dialogue));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetStar(); },
        Vector2(mWindowWidth * 1.5f, mMap->GetHeight() - mWindowHeight * 1.2f),
        320.0f));
    std::vector<std::string> dialogue2 = {
        "Zoe: Espere! Volte aqui!",
        "Zoe: Onde sera que ela foi? Preciso saber se ela está me levando para algum lugar..."};
    steps.push_back(std::make_unique<DialogueStep>(this, dialogue2));
    steps.push_back(std::make_unique<UnspawnStep>(this, [this]()
                                                  { return GetStar(); }));

    AddCutscene("Intro",
                std::move(steps),
                [this]() {});

    StartCutscene("Intro");
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    // Reset camera position
    mCameraPos.Set(0.0f, 0.0f);

    // Reset gameplay state
    mGamePlayState = GamePlayState::Playing;

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE);

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
        LoadMainMenu();
    else if (mNextScene == GameScene::Level1)
        LoadFirstLevel();

    // Set new scene
    mGameScene = mNextScene;
}

void Game::LoadMainMenu()
{
    UIScreen *mainMenu = new UIScreen(this, "../assets/Fonts/VT323-Regular.ttf");

    mainMenu->AddBackground(
        "../assets/Sprites/Menu/background.png",
        Vector2(0, 0),
        Vector2(mWindowWidth, mWindowHeight));

    const Vector2 playButtonSize = Vector2(
        mWindowWidth / 6.0f,
        mWindowHeight / 12.0f);

    const Vector2 playButtonPos = Vector2(
        mWindowWidth / 2.0f - playButtonSize.x / 2.0f,
        mWindowHeight / 2.0f - playButtonSize.y / 2.0f);

    mainMenu->AddButton(
        "Play",
        playButtonPos,
        playButtonSize,
        [this]()
        { SetGameScene(GameScene::Level1); },
        Vector2(CHAR_WIDTH * 4, WORD_HEIGHT));

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            Quit();
            break;

        case SDL_KEYDOWN:
            // if clicks G, activate debug mode
            if (event.key.keysym.sym == SDLK_g && event.key.repeat == 0)
            {
                mDebugMode = !mDebugMode;
                if (mDebugMode)
                    std::cout << "Debug mode activated" << std::endl;
                else
                    std::cout << "Debug mode deactivated" << std::endl;
            }

            // Handle key press for UI screens
            if (!mUIStack.empty())
            {
                mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
            }

            HandleKeyPressActors(event.key.keysym.sym, event.key.repeat == 0);

            // Check if the Return key has been pressed to pause/unpause the game
            if (event.key.keysym.sym == SDLK_RETURN)
            {
                TogglePause();
            }

            break;

        case SDL_MOUSEBUTTONDOWN:
            // Handle mouse click for UI screens
            if (!mUIStack.empty())
            {
                Vector2 logicalMouseClick = GetLogicalMousePos();
                mUIStack.back()->HandleMouseClick(
                    event.button.button,
                    logicalMouseClick.x,
                    logicalMouseClick.y);
            }
        }
    }

    ProcessInputActors();
}

void Game::ProcessInputActors()
{
    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    if (mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
            mCameraPos,
            mWindowWidth,
            mWindowHeight);

        for (auto actor : actorsOnCamera)
        {
            actor->ProcessInput(state);
        }
    }

    else if (mGamePlayState == GamePlayState::Dialogue)
    {
        GetDialogueSystem()->HandleInput(state);
    }
}

void Game::HandleKeyPressActors(const int key, const bool isPressed)
{
    if (mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
            mCameraPos,
            mWindowWidth,
            mWindowHeight);

        for (auto actor : actorsOnCamera)
        {
            actor->HandleKeyPress(key, isPressed);
        }
    }
}

void Game::TogglePause()
{
    if (mGameScene != GameScene::MainMenu)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mGamePlayState = GamePlayState::Paused;
            // mAudio->PlaySound("Coin.wav");
            mAudio->PauseSound(mMusicHandle);
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;
            // mAudio->PlaySound("Coin.wav");
            mAudio->ResumeSound(mMusicHandle);
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
        ; // Cap at 60 fps

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (mGamePlayState == GamePlayState::Playing || mGamePlayState == GamePlayState::PlayingCutscene)
    {
        UpdateActors(deltaTime);
    }

    if (mGamePlayState == GamePlayState::PlayingCutscene && mCurrentCutscene)
    {
        mCurrentCutscene->Update(deltaTime);
    }

    // Reinsert audio system
    mAudio->Update(deltaTime);

    // Reinsert UI screens
    for (auto ui : mUIStack)
    {
        if (ui->GetState() == UIScreen::UIState::Active)
        {
            ui->Update(deltaTime);
        }
    }

    // Delete any UIElements that are closed
    auto iter = mUIStack.begin();
    while (iter != mUIStack.end())
    {
        if ((*iter)->GetState() == UIScreen::UIState::Closing)
        {
            delete *iter;
            iter = mUIStack.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    if (mGameScene != GameScene::MainMenu)
    {
        if (mGamePlayState == GamePlayState::Playing)
        {
            mHUD->SetFPS(static_cast<int>(1.0f / deltaTime));
        }
    }

    UpdateSceneManager(deltaTime);
    UpdateCamera();

    mPrevDeltaTime = deltaTime;
}

void Game::UpdateSceneManager(float deltaTime)
{
    if (mSceneManagerState == SceneManagerState::Entering)
    {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0.0f)
        {
            mSceneManagerTimer = TRANSITION_TIME;
            mSceneManagerState = SceneManagerState::Active;
        }
    }
    else if (mSceneManagerState == SceneManagerState::Active)
    {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0.0f)
        {
            ChangeScene();
            mSceneManagerState = SceneManagerState::None;
        }
    }
}

void Game::UpdateCamera()
{
    if (!mZoe)
        return;

    float cameraX = mZoe->GetPosition().x - mWindowWidth / 2.0f;
    float cameraY = mZoe->GetPosition().y - mWindowHeight / 2.0f;
    float maxCameraX = mMap->GetWidth() - mWindowWidth;
    float maxCameraY = mMap->GetHeight() - mWindowHeight;

    mCameraPos.x = std::min(cameraX, maxCameraX);
    mCameraPos.y = std::min(cameraY, maxCameraY);

    mCameraPos.x = std::max(0.0f, mCameraPos.x);
    mCameraPos.y = std::max(0.0f, mCameraPos.y);
}

void Game::UpdateActors(float deltaTime)
{
    // Get actors on camera
    std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
        mCameraPos,
        mWindowWidth,
        mWindowHeight);

    bool isZoeOnCamera = false;
    for (auto actor : actorsOnCamera)
    {
        actor->Update(deltaTime);
        if (actor == mZoe)
        {
            isZoeOnCamera = true;
        }
    }

    // If Zoe is not on camera, update him (player should always be updated)
    if (!isZoeOnCamera && mZoe)
    {
        mZoe->Update(deltaTime);
    }

    for (auto actor : actorsOnCamera)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            delete actor;
            if (actor == mZoe)
            {
                mZoe = nullptr;
            }
        }
    }
}

void Game::AddActor(Actor *actor)
{
    mSpatialHashing->Insert(actor);
}

void Game::RemoveActor(Actor *actor)
{
    mSpatialHashing->Remove(actor);
}

void Game::Reinsert(Actor *actor)
{
    mSpatialHashing->Reinsert(actor);
}

std::vector<Actor *> Game::GetNearbyActors(const Vector2 &position, const int range)
{
    return mSpatialHashing->Query(position, range);
}

std::vector<AABBColliderComponent *> Game::GetNearbyColliders(const Vector2 &position, const int range)
{
    return mSpatialHashing->QueryColliders(position, range);
}

void Game::GenerateOutput()
{
    // Clear frame with background color
    SDL_SetRenderDrawColor(mRenderer, mBackgroundColor.x, mBackgroundColor.y, mBackgroundColor.z, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    // Draw background texture considering camera position
    if (mBackgroundTexture)
    {
        if (!mBackgroundIsCameraWise)
        {
            mBackgroundPosition.Set(mCameraPos.x, mCameraPos.y);
        }

        SDL_Rect dstRect = {
            static_cast<int>(mBackgroundPosition.x - mCameraPos.x),
            static_cast<int>(mBackgroundPosition.y - mCameraPos.y),
            static_cast<int>(mBackgroundSize.x),
            static_cast<int>(mBackgroundSize.y)};

        SDL_RenderCopy(mRenderer, mBackgroundTexture, nullptr, &dstRect);
    }

    // Get actors on camera
    std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
        mCameraPos,
        mWindowWidth,
        mWindowHeight);

    // Get list of drawables in draw order
    std::vector<DrawComponent *> drawables;
    for (auto actor : actorsOnCamera)
    {
        std::vector<DrawComponent *> actorDrawables = actor->GetComponents<DrawComponent>();

        for (auto drawable : actorDrawables)
        {
            if (drawable && drawable->IsVisible())
            {
                drawables.emplace_back(drawable);
            }
        }
    }

    // Sort drawables by draw order
    std::sort(
        drawables.begin(),
        drawables.end(),
        [](const DrawComponent *a, const DrawComponent *b)
        {
            return a->GetDrawOrder() < b->GetDrawOrder();
        });

    // Draw all drawables
    for (auto drawable : drawables)
    {
        drawable->Draw(mRenderer, mModColor);
    }

    // Draw all UI screens
    for (auto ui : mUIStack)
    {
        ui->Draw(mRenderer);
    }

    // Draw transition rect
    if (mSceneManagerState == SceneManagerState::Active)
    {
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
        SDL_Rect rect = {0, 0, mWindowWidth, mWindowHeight};
        SDL_RenderFillRect(mRenderer, &rect);
    }

    SDL_RenderSetLogicalSize(mRenderer, mRealWindowWidth, mRealWindowHeight);

    // draw dialogue system without scaling to avoid blurriness
    mDialogueSystem->Draw(mRenderer);

    if (SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight) != 0)
    {
        throw std::runtime_error("Failed to set logical size: " + std::string(SDL_GetError()));
    }

    if (!mDebugMode)
    {
        // Swap front buffer and back buffer
        SDL_RenderPresent(mRenderer);
        return;
    }

    // draw collider boxes only if the player has collider
    for (auto actor : actorsOnCamera)
    {
        std::vector<AABBColliderComponent *> colliders = actor->GetComponents<AABBColliderComponent>();
        for (auto collider : colliders)
        {
            if (collider)
            {
                SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
                Vector2 min = collider->GetMin();
                Vector2 max = collider->GetMax();
                SDL_Rect rect = {
                    static_cast<int>(min.x - mCameraPos.x),
                    static_cast<int>(min.y - mCameraPos.y),
                    static_cast<int>(max.x - min.x),
                    static_cast<int>(max.y - min.y)};
                SDL_RenderDrawRect(mRenderer, &rect);
            }
        }
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

void Game::SetBackgroundImage(
    const std::string &texturePath, const Vector2 &position,
    const Vector2 &size, bool isCameraWise)
{
    if (mBackgroundTexture)
    {
        SDL_DestroyTexture(mBackgroundTexture);
        mBackgroundTexture = nullptr;
    }

    // Load background texture
    mBackgroundTexture = LoadTexture(texturePath);
    if (!mBackgroundTexture)
    {
        SDL_Log("Failed to load background texture: %s", texturePath.c_str());
    }

    mBackgroundIsCameraWise = isCameraWise;

    // Set background position
    if (mBackgroundIsCameraWise)
    {
        mBackgroundPosition.Set(position.x, position.y);
    }
    else
    {
        mBackgroundPosition.Set(mCameraPos.x, mCameraPos.y);
    }

    // Set background size
    mBackgroundSize.Set(size.x, size.y);
}

SDL_Texture *Game::LoadTexture(const std::string &texturePath)
{
    SDL_Surface *surface = IMG_Load(texturePath.c_str());

    if (!surface)
    {
        SDL_Log("Failed to load image: %s", IMG_GetError());
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface);

    if (!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }

    return texture;
}

UIFont *Game::LoadFont(const std::string &fileName)
{
    auto iter = mFonts.find(fileName);
    if (iter != mFonts.end())
    {
        return iter->second;
    }
    else
    {
        UIFont *font = new UIFont(mRenderer);
        if (font->Load(fileName))
        {
            mFonts.emplace(fileName, font);
        }
        else
        {
            font->Unload();
            delete font;
            font = nullptr;
        }
        return font;
    }
}

void Game::UnloadScene()
{
    // Delete actors
    delete mSpatialHashing;

    // Delete UI screens
    for (auto ui : mUIStack)
    {
        delete ui;
    }
    mUIStack.clear();

    // Delete background texture
    if (mBackgroundTexture)
    {
        SDL_DestroyTexture(mBackgroundTexture);
        mBackgroundTexture = nullptr;
    }

    if (mMap)
    {
        delete mMap;
        mMap = nullptr;
    }
}

void Game::Shutdown()
{
    UnloadScene();

    for (auto font : mFonts)
    {
        font.second->Unload();
        delete font.second;
    }
    mFonts.clear();

    delete mAudio;
    mAudio = nullptr;

    Mix_CloseAudio();

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

Vector2 Game::GetLogicalMousePos() const
{
    float lx, ly;
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_RenderWindowToLogical(mRenderer, (float)x, (float)y, &lx, &ly);
    return Vector2(lx, ly);
}

void Game::AddCutscene(const std::string &name, std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback)
{
    if (mCutscenes.find(name) != mCutscenes.end())
    {
        SDL_Log("Cutscene with name '%s' already exists. Overwriting.", name.c_str());
        delete mCutscenes[name];
    }

    Cutscene *newCutscene = new Cutscene(std::move(steps), onCompleteCallback, this);
    mCutscenes.emplace(name, std::move(newCutscene));
}

void Game::StartCutscene(const std::string &name)
{
    auto iter = mCutscenes.find(name);
    if (iter != mCutscenes.end())
    {
        mCurrentCutscene = iter->second;
        mGamePlayState = GamePlayState::PlayingCutscene;
        mCurrentCutscene->Play();
        return;
    }

    throw std::runtime_error("Cutscene with name '" + name + "' not found.");
}

void Game::PauseCutscene()
{
    if (mCurrentCutscene)
    {
        mCurrentCutscene->Pause();
        mGamePlayState = GamePlayState::Paused;
    }
}

void Game::ResetCutscenes()
{
    for (auto &pair : mCutscenes)
        delete pair.second;

    mCutscenes.clear();
    mCurrentCutscene = nullptr;

    if (mGamePlayState == GamePlayState::PlayingCutscene)
        mGamePlayState = GamePlayState::Playing;
}

bool Game::ActorOnCamera(Actor *actor)
{
    if (!actor)
        return false;

    Vector2 actorPos = actor->GetPosition();

    // Check if actor's bounding box intersects with camera's bounding box
    if (actorPos.x < mCameraPos.x ||                // Actor is to the left of camera
        actorPos.x > mCameraPos.x + mWindowWidth || // Actor is to the right of camera
        actorPos.y < mCameraPos.y ||                // Actor is above camera
        actorPos.y > mCameraPos.y + mWindowHeight)  // Actor is below camera
    {
        return false; // No intersection
    }

    return true; // Intersection exists
}

int Game::GetMapWidth()
{
    if (mMap == nullptr)
        return 0.f;
    return mMap->GetWidth();
}

int Game::GetMapHeight()
{
    if (mMap == nullptr)
        return 0.f;
    return mMap->GetHeight();
}