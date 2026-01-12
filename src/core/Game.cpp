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
#include "../components/collider/AABBColliderComponent.h"
#include "../ui/DialogueSystem.h"
#include "../actors/Star.h"
#include "../actors/Enemy.h"
#include "../actors/Portal.h"

Game::Game()
    : mWindow(nullptr), mRenderer(nullptr), mTicksCount(0), mIsRunning(true),
      mZoe(nullptr), mHUD(nullptr), mBackgroundColor(0, 0, 0),
      mModColor(255, 255, 255), mCameraPos(Vector2::Zero), mAudio(nullptr),
      mSceneManagerTimer(0.0f), mSceneManagerState(SceneManagerState::None), mGameScene(GameScene::MainMenu),
      mNextScene(GameScene::Level1), mBackgroundTexture(nullptr), mBackgroundSize(Vector2::Zero),
      mBackgroundPosition(Vector2::Zero), mMap(nullptr), mBackgroundIsCameraWise(true),
      mCurrentCutscene(nullptr), mCutscenes(), mGamePlayState(GamePlayState::Playing),
      mDebugMode(false), mEnemies(), mStar(nullptr), mApplyGravityScene(true),
      mCameraCenter(CameraCenter::Zoe), mMaintainCameraInMap(true), mCameraCenterPos(Vector2::Zero),
      mController(nullptr), mMustAlwaysUpdateActors(), mPreviousGameState(GamePlayState::Playing),
      mRealWindowHeight(0), mRealWindowWidth(0)
{
    mWindowWidth = 640;
    mWindowHeight = 352;

    mDialogueSystem = new DialogueSystem(mGamePlayState);
}

void Game::SetMap(const std::string &path)
{
    mMap = new Map(this, path);
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);

    mRealWindowWidth = mode.w;
    mRealWindowHeight = mode.h;

    mWindow = SDL_CreateWindow(
        "astral",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mRealWindowWidth, mRealWindowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderSetIntegerScale(mRenderer, SDL_TRUE))
    {
        SDL_Log("Failed to set integer scale: %s", SDL_GetError());
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

    int mappingsAdded = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
    if (mappingsAdded == -1) {
        SDL_Log("Warning: gamecontrollerdb.txt not found! Generic controllers might not work.");
    } else {
        SDL_Log("Success: Loaded %d controller mappings.", mappingsAdded);
    }

    // Start random number generator
    Random::Init();

    mConfig = new Config();
    mConfig->Initialize("config.json");

    // Initialize game systems
    mAudio = new AudioSystem();
    mSpatialHashing = new SpatialHashing(TILE_SIZE,
                                         LEVEL_WIDTH * TILE_SIZE,
                                         LEVEL_HEIGHT * TILE_SIZE);

    mDialogueSystem->Initialize(this);

    SetGameScene(GameScene::MainMenu);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    mTicksCount = SDL_GetTicks();

    mAudio->CacheAllSounds();

    return true;
}

// Scene Management:

void Game::SetGameScene(Game::GameScene scene, float sceneLeftTime)
{
    if (mSceneManagerState != SceneManagerState::None)
        return;

    mNextScene = scene;
    mSceneManagerState = SceneManagerState::Entering;
    mSceneManagerTimer = sceneLeftTime;
}

void Game::ResetGameScene(float transitionTime)
{
    SetGameScene(mGameScene, transitionTime);
}

void Game::ChangeScene()
{
    // Unload current Scene
    UnloadScene();

    // Reset camera position
    mCameraPos.Set(0.0f, 0.0f);
    SetMaintainCameraInMap(true);
    SetCameraCenterToZoe();

    // Reset gameplay state
    mGamePlayState = GamePlayState::Playing;

    mAudio->StopAllSounds();

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(TILE_SIZE, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE);

    SetApplyGravityScene(Game::APPLY_GRAVITY_SCENE_DEFAULT);

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
        LoadMainMenu();
    else if (mNextScene == GameScene::Bedroom)
        LoadBedroom();
    else if (mNextScene == GameScene::BedroomPortal)
        LoadBedroomPortal();
    else if (mNextScene == GameScene::Level1)
        LoadFirstLevel();
    else if (mNextScene == GameScene::DeathScreen)
        LoadDeathScreen();
    else if (mNextScene == GameScene::EndDemo)
        LoadEndDemoScene();
    else if (mNextScene == GameScene::Tests)
        LoadTestsLevel();

    // Set new scenes
    mGameScene = mNextScene;
}

void Game::UpdateSceneManager(float deltaTime)
{
    if (mSceneManagerState == SceneManagerState::Entering)
    {
        mSceneManagerTimer -= deltaTime;
        if (mSceneManagerTimer <= 0.0f)
        {
            mSceneManagerTimer = 0.f;
            mSceneManagerState = SceneManagerState::Active;
        }
    }
    else if (mSceneManagerState == SceneManagerState::Active)
    {
        mSceneManagerTimer += deltaTime;
        if (mSceneManagerTimer >= TRANSITION_TIME_BETWEEM_SCENES)
        {
            ChangeScene();
            mSceneManagerState = SceneManagerState::None;
        }
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

//

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
    int numJoysticks = SDL_NumJoysticks();

    for (int i = 0; i < numJoysticks; ++i)
    {
        if (SDL_IsGameController(i))
        {
            mController = SDL_GameControllerOpen(i);
            if (!mController)
            {
                SDL_Log("Could not open gamecontroller %d: %s", i, SDL_GetError());
            }
            break;
        }
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            Quit();
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_g && event.key.repeat == 0)
            {
                mDebugMode = !mDebugMode;
                if (mDebugMode) std::cout << "Debug mode activated" << std::endl;
            }

            // Handle key press for UI screens
            if (!mUIStack.empty())
            {
                mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
            }

            HandleKeyPressActors(event.key.keysym.sym, event.key.repeat == 0);

            // Check if the Return key has been pressed to pause/unpause the game
            if (event.key.keysym.sym == SDLK_RETURN && 
                GetGamePlayState() == GamePlayState::Playing)
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

    if (mGamePlayState == GamePlayState::Playing)
    {
        ProcessInputActors();
    }

    if (mGamePlayState == GamePlayState::Dialogue)
    {
        const Uint8 *state = SDL_GetKeyboardState(nullptr);
        GetDialogueSystem()->HandleInput(state);
    }
}

void Game::ProcessInputActors()
{
    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    std::vector<Actor *> toProcessActors = mMustAlwaysUpdateActors;

    // Get actors on camera
    std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
        mCameraPos,
        mWindowWidth,
        mWindowHeight);

    for (auto actor : actorsOnCamera)
    {
        // Avoid duplicates
        if (std::find(toProcessActors.begin(), toProcessActors.end(), actor) == toProcessActors.end())
        {
            toProcessActors.push_back(actor);
        }
    }

    for (auto actor : toProcessActors)
    {
        actor->ProcessInput(state);
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
    if (mGameScene == GameScene::MainMenu)
        return;

    if (mGamePlayState == GamePlayState::Playing)
    {
        mGamePlayState = GamePlayState::Paused;
        mAudio->PauseSound(mMusicHandle);
    }

    else if (mGamePlayState == GamePlayState::Paused)
    {
        mGamePlayState = GamePlayState::Playing;
        mAudio->ResumeSound(mMusicHandle);
    }
}

void Game::UpdateGame()
{
    // Cap at 60 fps
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
    {
    };

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (mGamePlayState == GamePlayState::Playing ||
        mGamePlayState == GamePlayState::PlayingCutscene)
    {
        UpdateActors(deltaTime);
    }

    if (mGamePlayState == GamePlayState::PlayingCutscene && mCurrentCutscene)
    {
        mCurrentCutscene->Update(deltaTime);
    }

    mAudio->Update(deltaTime);

    if (mGamePlayState == GamePlayState::Dialogue)
    {
        GetDialogueSystem()->Update(deltaTime);
    }

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

    if (mGameScene != GameScene::MainMenu && mGameScene != GameScene::DeathScreen && mHUD && mZoe)
    {
        mHUD->SetFPS(static_cast<int>(1.0f / deltaTime));
        mHUD->SetLife(mZoe->GetLifes());
        mHUD->SetLoadingBarProgress(
            mZoe->CheckFireballOnCooldown(),
            mZoe->GetFireballCooldownProgress());
    }

    UpdateSceneManager(deltaTime);
    UpdateCamera();

    EndDemoCheck();
}

void Game::UpdateCamera()
{
    if (mCameraCenter == CameraCenter::Zoe && !mZoe)
        return;

    if (mMap == nullptr)
        return;


    Vector2 center;
    switch (mCameraCenter)
    {
    case CameraCenter::Zoe:
        center = mZoe->GetCenter();
        break;
    case CameraCenter::Point:
        center = mCameraCenterPos;
        break;
    case CameraCenter::LogicalWindowSizeCenter: {
            if (!mZoe) 
            {
                throw std::runtime_error("Zoe actor is null when trying to center camera to logical window size center.");
            }
            
            center = GetBoxCenter(
                mZoe->GetCenter(),
                static_cast<float>(mWindowWidth), 
                static_cast<float>(mWindowHeight));
            break;
        }
    }

    float cameraX = center.x - mWindowWidth / 2.0f;
    float cameraY = center.y - mWindowHeight / 2.0f;
    float maxCameraX = mMap->GetWidth() - mWindowWidth;
    float maxCameraY = mMap->GetHeight() - mWindowHeight;

    if (!mMaintainCameraInMap)
    {
        mCameraPos.x = cameraX;
        mCameraPos.y = cameraY;

        mCameraPos.x = mCameraPos.x;
        mCameraPos.y = mCameraPos.y;
        return;
    };

    mCameraPos.x = std::min(cameraX, maxCameraX);
    mCameraPos.y = std::min(cameraY, maxCameraY);

    mCameraPos.x = std::max(0.0f, mCameraPos.x);
    mCameraPos.y = std::max(0.0f, mCameraPos.y);
}

void Game::UpdateActors(float deltaTime)
{
    std::vector<Actor *> toUpdateActors = mMustAlwaysUpdateActors;

    // Get actors on camera
    std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
        mCameraPos,
        mWindowWidth,
        mWindowHeight,
        Game::TILE_SIZE * 2.f);

    for (auto actor : actorsOnCamera)
    {
        // Avoid duplicates
        if (std::find(toUpdateActors.begin(), toUpdateActors.end(), actor) == toUpdateActors.end())
        {
            toUpdateActors.push_back(actor);
        }
    }

    for (auto actor : toUpdateActors)
    {
        actor->Update(deltaTime);
    }

    for (auto actor : toUpdateActors)
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

    auto it = std::find(
        mMustAlwaysUpdateActors.begin(),
        mMustAlwaysUpdateActors.end(),
        actor);

    if (it != mMustAlwaysUpdateActors.end())
    {
        mMustAlwaysUpdateActors.erase(it);
    }
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

void Game::DrawDebugInfo(std::vector<Actor *> &actorsOnCamera)
{
    mSpatialHashing->Draw(mRenderer, mCameraPos, mWindowWidth, mWindowHeight);

    // draw collider boxes only if the player has collider
    for (auto actor : actorsOnCamera)
    {
        std::vector<AABBColliderComponent *> colliders = actor->GetComponents<AABBColliderComponent>();
        for (auto collider : colliders)
        {
            if (collider && collider->IsEnabled())
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

        // check actor type if it's enemy
        if (auto enemy = dynamic_cast<Enemy *>(actor))
        {
            SDL_SetRenderDrawColor(mRenderer, 114, 16, 199, 255);
            std::vector<SDL_Rect> path = enemy->GetPath();

            while (path.size() > 0)
            {
                SDL_Rect rect = path.back();
                rect.x -= static_cast<int>(mCameraPos.x);
                rect.y -= static_cast<int>(mCameraPos.y);
                SDL_RenderDrawRect(mRenderer, &rect);
                path.pop_back();
            }

            // draw enemy's current applied force vector
            Vector2 force = enemy->GetCurrentAppliedForce(.05f);
            Vector2 enemyPos = enemy->GetCenter();
            SDL_SetRenderDrawColor(mRenderer, 29, 88, 146, 255);
            SDL_RenderDrawLine(
                mRenderer,
                static_cast<int>(enemyPos.x - mCameraPos.x),
                static_cast<int>(enemyPos.y - mCameraPos.y),
                static_cast<int>(enemyPos.x + force.x - mCameraPos.x),
                static_cast<int>(enemyPos.y + force.y - mCameraPos.y));
        }
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
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
        mWindowHeight,
        Game::TILE_SIZE * 2.f);

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

    if (SDL_RenderSetLogicalSize(mRenderer, mRealWindowWidth, mRealWindowHeight) != 0)
    {
        throw std::runtime_error("Failed to set real size: " + std::string(SDL_GetError()));
    }

    // draw dialogue system without scaling to avoid blurriness
    mDialogueSystem->Draw(mRenderer);

    if (SDL_RenderSetLogicalSize(mRenderer, mWindowWidth, mWindowHeight) != 0)
    {
        throw std::runtime_error("Failed to set logical size: " + std::string(SDL_GetError()));
    }

    // Draw transition rect above everything
    if (mSceneManagerState == SceneManagerState::Active)
    {
        Uint8 alpha = static_cast<Uint8>((mSceneManagerTimer / TRANSITION_TIME_BETWEEM_SCENES) * 255);
        SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, alpha);

        SDL_Rect rect = {0, 0, mWindowWidth, mWindowHeight};
        SDL_RenderFillRect(mRenderer, &rect);
    }

    if (!mDebugMode)
    {
        SDL_RenderPresent(mRenderer);
        return;
    }

    DrawDebugInfo(actorsOnCamera);
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

    if (mController)
    {
        SDL_GameControllerClose(mController);
        mController = nullptr;
    }

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
    return Vector2(lx, ly) + mCameraPos;
}

void Game::AddCutscene(const std::string &name, std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback)
{
    auto it = mCutscenes.find(name);
    if (it != mCutscenes.end())
    {
        SDL_Log("Cutscene with name '%s' already exists. Overwriting.", name.c_str());
        delete it->second;
        mCutscenes.erase(it);
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

void Game::AddEnemy(class Enemy *enemy)
{
    mEnemies.emplace_back(enemy);
}

void Game::RemoveEnemy(class Enemy *enemy)
{
    auto iter = std::find(mEnemies.begin(), mEnemies.end(), enemy);
    if (iter != mEnemies.end())
    {
        mEnemies.erase(iter);
    }
}

void Game::SetZoe(class Zoe *zoe)
{
    if (mZoe != nullptr)
    {
        mZoe->SetState(ActorState::Destroy);
    }
    mZoe = zoe;
}

Vector2 Game::GetBoxCenter(const Vector2& pos, float boxW, float boxH)
{
    // Determine which box the point is in
    int boxX = static_cast<int>(pos.x / boxW);
    int boxY = static_cast<int>(pos.y / boxH);

    // Compute center of that box
    float centerX = boxX * boxW + boxW * 0.5f;
    float centerY = boxY * boxH + boxH * 0.5f;

    return Vector2(centerX, centerY);
}

std::vector<class Enemy *> Game::GetEnemies(const Vector2 &min, const Vector2 &max)
{
    std::vector<Enemy*> nearbyEnemies;
    for (auto enemy : mEnemies)
    {
        Vector2 enemyPos = enemy->GetCenter();
        if (enemyPos.x >= min.x && enemyPos.x <= max.x &&
            enemyPos.y >= min.y && enemyPos.y <= max.y)
        {
            nearbyEnemies.push_back(enemy);
        }
    }
    return nearbyEnemies;
}

void Game::EndDemoCheck()
{
    if (GameScene::Level1 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    if (isEnding)
    {
        return;
    }

    Vector2 start = Vector2(0.f, 0.f);
    Vector2 end = Vector2(630.f, 340.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
    {
        return;
    }

    if (zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    {
        StartCutscene("endDemo");
        isEnding = true;
    }
}