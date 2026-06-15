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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
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
#include "../components/ai/AIMovementComponent.h"
#include "../actors/Portal.h"
#include "../actors/enemies/Zod.h"
#include "../actors/Item.h"
#include "../actors/enemies/Zathura.h"

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
      mRealWindowHeight(0), mRealWindowWidth(0), mDeltatime(0.f), mShakeCounter(0.f), mShakeIntensity(3.f),
      mPortal(nullptr), mIsPhysicsFrozen(false), mHasSpawnedPortalLevel2(false),
      mMetalCratePortionTimeCounter(0.f), mQuasarEncounterTimeCounter(0.f), mZathura(nullptr),
      mLastUnTooglePauseTick(0)
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

#ifdef __EMSCRIPTEN__
    // On web, SDL_GetCurrentDisplayMode returns the monitor resolution, which is
    // larger than the browser viewport. Creating a canvas at monitor resolution
    // causes a CSS/viewport mismatch that makes the game invisible until the user
    // changes browser zoom. Use the actual viewport size instead.
    mRealWindowWidth  = EM_ASM_INT({ return window.innerWidth;  });
    mRealWindowHeight = EM_ASM_INT({ return window.innerHeight; });
    if (mRealWindowWidth  <= 0) mRealWindowWidth  = 1280;
    if (mRealWindowHeight <= 0) mRealWindowHeight = 720;
#else
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    mRealWindowWidth = mode.w;
    mRealWindowHeight = mode.h;
#endif

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
    mIsPhysicsFrozen = false;

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
    else if (mNextScene == GameScene::Level2)
        LoadSecondLevel();
    else if (mNextScene == GameScene::DeathScreen)
        LoadDeathScreen();
    else if (mNextScene == GameScene::EndDemo)
        LoadEndDemoScene();
    else if (mNextScene == GameScene::Tests)
        LoadTestsLevel();
    else if (mNextScene == GameScene::BedroomFinal)
        LoadBedroomFinal();

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
    mZoe = nullptr;
    mStar = nullptr;
    mPortal = nullptr;
    mZathura = nullptr;
    mEnemies.clear();

    // Delete UI screens - HUD is here
    for (auto ui : mUIStack)
    {
        delete ui;
    }
    mUIStack.clear();

    mHUD = nullptr;

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

    SDL_GameController *controller = GetController();

    // Check if the Return key has been pressed to pause/unpause the game
    if (
        SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) &&
        (GetGamePlayState() == GamePlayState::Playing || 
        GetGamePlayState() == GamePlayState::Paused)
    )
    {
        UnTogglePause();
    }

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) && !mUIStack.empty())
    {
        UICursor *cursor = mUIStack.back()->GetCursor();
        if (cursor) 
        {
            mUIStack.back()->HandleControllerButtonA(cursor->GetCenter());
        }
    }

    std::vector<SDL_Event> events;
    while (SDL_PollEvent(&events.emplace_back()))
    {
    }

    for (const auto &event : events)
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

            switch (event.key.keysym.sym) {
                case SDLK_z:
                    new Zod(this, Vector2(310.f, 120.f) + GetCameraPos());
                    break;
                
                case SDLK_s:
                    new Sith(this, Vector2(310.f, 120.f) + GetCameraPos());
                    break;
                
                case SDLK_q:
                    new Quasar(this, Vector2(310.f, 120.f) + GetCameraPos());
                    break;

                case SDLK_n: {
                    Vector2 zoePos = GetZoe()->GetPosition();
                    Item::CreateNevascaItem(this, zoePos + Vector2(0.f, -40.f));
                    break;
                }
                
                case SDLK_v: {
                    Vector2 zoePos = GetZoe()->GetPosition();
                    Item::CreateVentaniaItem(this, zoePos + Vector2(0.f, -40.f));
                    break;
                }

                case SDLK_m: {
                    Vector2 zoePos = GetZoe()->GetPosition();
                    Item::CreateFireballItem(this, zoePos + Vector2(0.f, -40.f));
                    break;
                }

                case SDLK_l:
                    GetZoe()->SetCenter(Vector2(1330.f, 655.f));
                    break;
            }

            break;
        case SDL_MOUSEBUTTONDOWN:
            break;
        }
    }

    if (mGamePlayState == GamePlayState::PlayingCutscene && mCurrentCutscene)
    {
        mCurrentCutscene->OnProcessInput(events);
    }

    if (mGamePlayState == GamePlayState::Playing)
    {
        ProcessInputActors(events);
    }

    if (mGamePlayState == GamePlayState::Dialogue)
    {
        const Uint8 *state = SDL_GetKeyboardState(nullptr);
        GetDialogueSystem()->HandleInput(state);
    }
}

void Game::ProcessInputActors(const std::vector<SDL_Event>& events)
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
        actor->ProcessInput(state, events);
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

void Game::UnTogglePause()
{
    if ( // menus
        mGameScene == GameScene::MainMenu ||
        mGameScene == GameScene::DeathScreen ||
        mGameScene == GameScene::EndDemo
    )
        return;

    if (
        !SDL_TICKS_PASSED(SDL_GetTicks(), mLastUnTooglePauseTick + 500)
    ) 
    {
        return;
    }

    mLastUnTooglePauseTick = SDL_GetTicks();

    if (mGamePlayState == GamePlayState::Paused)
    {
        mGamePlayState = GamePlayState::Playing;
        mAudio->ResumeSound(mMusicHandle);
        return;
    }

    mGamePlayState = GamePlayState::Paused;
    mAudio->PauseSound(mMusicHandle);
}

void Game::UpdateGame()
{
    // Cap at 60 fps
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
    {
    };

    mDeltatime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (mDeltatime > 0.05f)
    {
        mDeltatime = 0.05f;
    }
    else if (mDeltatime < 0.0167) {
        mDeltatime = 0.0167f;
    }

    mTicksCount = SDL_GetTicks();

    if (mGamePlayState == GamePlayState::Playing ||
        mGamePlayState == GamePlayState::PlayingCutscene)
    {
        UpdateActors(mDeltatime);
    }

    if (mGamePlayState == GamePlayState::PlayingCutscene && mCurrentCutscene)
    {
        mCurrentCutscene->Update(mDeltatime);
    }

    mAudio->Update(mDeltatime);

    if (mGamePlayState == GamePlayState::Dialogue)
    {
        GetDialogueSystem()->Update(mDeltatime);
    }

    // Reinsert UI screens
    for (auto ui : mUIStack)
    {
        if (ui->GetState() == UIScreen::UIState::Active)
        {
            ui->Update(mDeltatime);
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
        mHUD->Update(mDeltatime);
    }

    UpdateSceneManager(mDeltatime);
    UpdateCamera();

    if (mGameScene == GameScene::Level1)
    {
        QuasarEncounterLevelCheck();
        BreakTilesFirstLevelCheck();
        LastPartFirstLevelCheck();
    }
    else if (mGameScene == GameScene::Level2)
    {
        SithEncounterBreakTilesCheck();
        SithEncounterBreakTilesCheck2();
        CheckPortalSpawn();
        CheckMetalCrateLevel();
    }
}

void Game::UpdateCamera()
{
    if (mCameraCenter == CameraCenter::Zoe && !mZoe)
        return;

    if (mMap == nullptr)
        return;

    Vector2 shakeOffset = Vector2::Zero;
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
                break;
            }

            center = GetBoxCenter(
                mZoe->GetCenter(),
                static_cast<float>(mWindowWidth),
                static_cast<float>(mWindowHeight));
            break;
        }
        case CameraCenter::Shaking: {
            if (!mZoe)
            {
                break;
            }

            center = GetBoxCenter(
                mZoe->GetCenter(),
                static_cast<float>(mWindowWidth),
                static_cast<float>(mWindowHeight));
            
            shakeOffset = Vector2(
                Math::RandRangeInt(-mShakeIntensity, mShakeIntensity),
                Math::RandRangeInt(-mShakeIntensity, mShakeIntensity)
            );

            mShakeCounter -= mDeltatime;

            if (mShakeCounter <= 0.f)
            {
                SetCameraCenterToLogicalWindowSizeCenter();
            }
            break;
        }
    }

    float cameraX = center.x - mWindowWidth / 2.0f;
    float cameraY = center.y - mWindowHeight / 2.0f;
    float maxCameraX = mMap->GetWidth() - mWindowWidth;
    float maxCameraY = mMap->GetHeight() - mWindowHeight;

    if (!mMaintainCameraInMap)
    {
        mCameraPos.x = cameraX + shakeOffset.x;
        mCameraPos.y = cameraY + shakeOffset.y;
        return;
    };

    mCameraPos.x = std::min(cameraX, maxCameraX);
    mCameraPos.y = std::min(cameraY, maxCameraY);

    mCameraPos.x = std::max(0.0f, mCameraPos.x);
    mCameraPos.y = std::max(0.0f, mCameraPos.y);

    // Apply shake offset after clamping so it's always visible
    mCameraPos.x += shakeOffset.x;
    mCameraPos.y += shakeOffset.y;
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
    // mSpatialHashing->Draw(mRenderer, mCameraPos, mWindowWidth, mWindowHeight);

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

            // draw enemy's current applied force vector
            Vector2 force = enemy->GetCurrentVelocity(2.f);
            Vector2 enemyPos = enemy->GetCenter();
            SDL_SetRenderDrawColor(mRenderer, 29, 88, 146, 255);
            SDL_RenderDrawLine(
                mRenderer,
                static_cast<int>(enemyPos.x - mCameraPos.x),
                static_cast<int>(enemyPos.y - mCameraPos.y),
                static_cast<int>(enemyPos.x + force.x - mCameraPos.x),
                static_cast<int>(enemyPos.y + force.y - mCameraPos.y));

            SDL_Rect threatRect = enemy->GetThreatRect();
            SDL_Rect rect = {
                static_cast<int>(threatRect.x - mCameraPos.x),
                static_cast<int>(threatRect.y - mCameraPos.y),
                static_cast<int>(threatRect.w),
                static_cast<int>(threatRect.h)};

            SDL_SetRenderDrawColor(mRenderer, 114, 16, 199, 255);
            SDL_RenderDrawRect(mRenderer, &rect);

            std::vector<Vector2> obstaclesAroundCenters = enemy->GetObstaclesAroundCenters();
            for (const auto& obstacleCenter : obstaclesAroundCenters)
            {
                SDL_Rect obstacleRect = {
                    static_cast<int>(obstacleCenter.x - 5 - mCameraPos.x),
                    static_cast<int>(obstacleCenter.y - 5 - mCameraPos.y),
                    10,
                    10
                };
                SDL_SetRenderDrawColor(mRenderer, 255, 165, 0, 255);
                SDL_RenderDrawRect(mRenderer, &obstacleRect);
            }
        
            float fovAngleDeg = enemy->GetFovAngle() * (180.0f / Math::Pi);
            Vector2 forward = enemy->GetForward();
            float maxDist = enemy->GetMaxSeeDistance();

            Vector2 leftEdge = Vector2::RotateVec(forward, -fovAngleDeg) * maxDist;
            Vector2 rightEdge = Vector2::RotateVec(forward, fovAngleDeg) * maxDist;

            SDL_SetRenderDrawColor(mRenderer, 0, 220, 0, 255);
            SDL_RenderDrawLine(mRenderer,
                static_cast<int>(enemyPos.x - mCameraPos.x),
                static_cast<int>(enemyPos.y - mCameraPos.y),
                static_cast<int>(enemyPos.x + leftEdge.x - mCameraPos.x),
                static_cast<int>(enemyPos.y + leftEdge.y - mCameraPos.y));
            SDL_RenderDrawLine(mRenderer,
                static_cast<int>(enemyPos.x - mCameraPos.x),
                static_cast<int>(enemyPos.y - mCameraPos.y),
                static_cast<int>(enemyPos.x + rightEdge.x - mCameraPos.x),
                static_cast<int>(enemyPos.y + rightEdge.y - mCameraPos.y));

            if (enemy->isAISeeking())
            {
                Vector2 seekIndicator = enemyPos + enemy->GetForward() * enemy->GetWidth() * 0.5f;
                SDL_Rect seekRect = {
                    static_cast<int>(seekIndicator.x - 4 - mCameraPos.x),
                    static_cast<int>(seekIndicator.y - 4 - mCameraPos.y),
                    8, 8
                };
                SDL_SetRenderDrawColor(mRenderer, 160, 32, 240, 255);
                SDL_RenderFillRect(mRenderer, &seekRect);
            }
        }
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);

    int componentsOnCamera = 0;
    for (auto actor : actorsOnCamera)
    {
        componentsOnCamera += static_cast<int>(actor->HowManyComponents());
    }
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

void Game::AddCutscene(const std::string &name, std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback, bool overwrite)
{
    auto it = mCutscenes.find(name);
    if (it != mCutscenes.end())
    {
        if (!overwrite)
        {
            SDL_Log("Cutscene with name '%s' already exists. Skipping.", name.c_str());
            return;
        }

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
        
        if (mCurrentCutscene->GetState() == Cutscene::State::Finished)
        {
            return;
        }

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

void Game::QuasarEncounterLevelCheck()
{
    if (GameScene::Level1 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    Vector2 start = Vector2(0.f, 0.f);
    Vector2 end = Vector2(630.f, 340.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    bool playerInArea = zoeCenter.x >= start.x && zoeCenter.x <= end.x 
        && zoeCenter.y >= start.y && zoeCenter.y <= end.y;

    if (!playerInArea)
    {
        return;
    }
    
    // if the only quasar is still full life.
    if (
        !enemiesInArea.empty() && 
        enemiesInArea[0]->GetLifes() == GetConfig()->Get<int>("QUASAR.HEALTH")
    )
    {
        mQuasarEncounterTimeCounter += GetDtLastFrame();

        if (mQuasarEncounterTimeCounter >= MAX_TIME_QUASAR_ENCOUNTER_TIP)
            StartCutscene("quasarEncounterTip");

        return;
    }

    if (enemiesInArea.empty())
    {
        StartCutscene("halfFirstLevel");
    }
}

void Game::BreakTilesFirstLevelCheck()
{
    if (GameScene::Level1 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    Vector2 start = Vector2(642.f, 712.f);
    Vector2 end = Vector2(1262.f, 1036.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
    {
        return;
    }

    if (zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    {
        StartCutscene("breakTiles");
    }
}

void Game::LastPartFirstLevelCheck()
{
    if (GameScene::Level1 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    Vector2 start = Vector2(1280.f, 712.f);
    Vector2 end = Vector2(1900.f, 1036.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
    {
        return;
    }

    if (zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    {
        StartCutscene("endFirstLevel");
    }
}

void Game::SithEncounterBreakTilesCheck()
{
    if (GameScene::Level2 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    Vector2 start = Vector2(13.f, 369.f);
    Vector2 end = Vector2(623.f, 690.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
    {
        return;
    }

    if (zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    {
        StartCutscene("breakLevelSithPhaseTiles");
    }
}

void Game::SithEncounterBreakTilesCheck2()
{
    if (GameScene::Level2 != mGameScene)
    {
        return;
    }

    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    Vector2 start = Vector2(655.f, 373.f);
    Vector2 end = Vector2(1270.f, 688.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
    {
        return;
    }

    if (zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    {
        StartCutscene("breakLevelSithPhaseTiles2");
    }
}

void Game::CheckPortalSpawn()
{
    if (mGameScene != GameScene::Level2)
        return;

    if (mGamePlayState != GamePlayState::Playing)
        return;

    if (mHasSpawnedPortalLevel2)
        return;

    Vector2 start = Vector2(656.f, 15.f);
    Vector2 end = Vector2(1265.f, 365.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    std::vector<Enemy *> enemiesInArea = GetEnemies(start, end);

    if (!enemiesInArea.empty())
        return;

    if (
        !(zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y)
    )
        return;

    Vector2 pos1 = Vector2(896, 64);
    Vector2 pos2 = Vector2(672, 160);
    Vector2 objSize = Vector2(128, 64);

    Vector2 realPos = (zoeCenter.x >= pos1.x && zoeCenter.x <= pos1.x + objSize.x &&
        zoeCenter.y >= pos1.y && zoeCenter.y <= pos1.y + objSize.y) ? pos2 : pos1;

    json parameters = json::object();
    parameters["cutscene_name"] = "spawnPortalLevel2";

    new Portal(
        this,
        realPos + (objSize * 0.5f)
    );

    new MapObject(
        this,
        SPAWN_PORTAL_LEVEL_2_OBJ_ID,
        "in",
        "play_cutscene",
        realPos,
        objSize,
        parameters);

    mHasSpawnedPortalLevel2 = true;
}

void Game::CheckMetalCrateLevel()
{
    if (mGameScene != GameScene::Level2)
        return;

    if (mGamePlayState != GamePlayState::Playing)
        return;

    if (mMetalCratePortionTimeCounter >= MAX_TIME_METAL_CRATE_TIP)
        return;

    // not the full level, just the tip portion.
    Vector2 start = Vector2(1474.f, 0.f);
    Vector2 end = Vector2(1686.f, 350.f);

    Vector2 zoeCenter = mZoe->GetCenter();
    RigidBodyComponent* zoeRigidBody = mZoe->GetComponent<RigidBodyComponent>();

    // might be gettting mechanic without tip
    if (!zoeRigidBody->GetOnGround())
        return;

    if (
        zoeCenter.x >= start.x && zoeCenter.x <= end.x &&
        zoeCenter.y >= start.y && zoeCenter.y <= end.y
    )
    {
        mMetalCratePortionTimeCounter += GetDtLastFrame();

        if (mMetalCratePortionTimeCounter >= MAX_TIME_METAL_CRATE_TIP)
            StartCutscene("metalCrateTip");
    }
}


Vector2 Game::getNormalizedControlerPad()
{
    SDL_GameController *controller = GetController();

    float padX = 0.0f;
    float padY = 0.0f;

    int rawX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    int rawY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

    const int DEADZONE = 2000;

    if (SDL_abs(rawX) < DEADZONE)
        rawX = 0;
    if (SDL_abs(rawY) < DEADZONE)
        rawY = 0;

    const float MAX_AXIS = 32767.0f;

    padX = rawX / MAX_AXIS;
    padY = rawY / MAX_AXIS;

    Vector2 normalizedPad = Vector2(padX, padY);
    normalizedPad.Normalize();

    return normalizedPad;
}

// -----

Checkpoint* Game::GetCurrentCheckpoint() const
{
    if (mZoe == nullptr)
    {
        return nullptr;
    }

    return mZoe->GetCurrentCheckpoint();
}

void Game::SetCheckpoint(const Vector2& position)
{
    if (mZoe == nullptr)
    {
        return;
    }

    mZoe->SetCheckpoint(position);
}