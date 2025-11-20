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

Game::Game(int windowWidth, int windowHeight)
    : mWindow(nullptr), mRenderer(nullptr), mTicksCount(0), mIsRunning(true),
      mZoe(nullptr), mHUD(nullptr), mBackgroundColor(0, 0, 0),
      mModColor(255, 255, 255), mCameraPos(Vector2::Zero), mAudio(nullptr),
      mSceneManagerTimer(0.0f), mSceneManagerState(SceneManagerState::None), mGameScene(GameScene::MainMenu),
      mNextScene(GameScene::Level1), mBackgroundTexture(nullptr), mBackgroundSize(Vector2::Zero),
      mBackgroundPosition(Vector2::Zero), mMap(nullptr), mBackgroundIsCameraWise(true),
      mCurrentCutscene(nullptr), mCutscenes(), mGamePlayState(GamePlayState::Playing),
      mDebugMode(false), mPrevDeltaTime(0.0f), mEnemies(), mStar(nullptr), mApplyGravityScene(true),
      mCameraCenter(CameraCenter::Zoe), mMaintainCameraInMap(true), mCameraCenterPos(Vector2::Zero),
      mController(nullptr)
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) != 0)
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

    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i)
    {
        if (SDL_IsGameController(i))
        {
            mController = SDL_GameControllerOpen(i);
            if (mController)
            {
                SDL_Log("Opened game controller %d: %s", i, SDL_GameControllerName(mController));
            }
            else
            {
                SDL_Log("Could not open gamecontroller %d: %s", i, SDL_GetError());
            }
            break;
        }
    }

    // Start random number generator
    Random::Init();

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

    // Set new scene
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

    GetDialogueSystem()->Update(deltaTime);

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

    if (mGameScene != GameScene::MainMenu && mGameScene != GameScene::DeathScreen && mHUD)
    {
        mHUD->SetFPS(static_cast<int>(1.0f / deltaTime));
    }

    UpdateSceneManager(deltaTime);
    UpdateCamera();

    mPrevDeltaTime = deltaTime;
}

void Game::UpdateCamera()
{
    if (mCameraCenter == CameraCenter::Zoe && !mZoe)
        return;

    if (mMap == nullptr)
        return;

    Vector2 center = (mCameraCenter == CameraCenter::Zoe) ? mZoe->GetPosition() : mCameraCenterPos;

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
    // Get actors on camera
    std::vector<Actor *> actorsOnCamera = mSpatialHashing->QueryOnCamera(
        mCameraPos,
        mWindowWidth,
        mWindowHeight,
        Game::TILE_SIZE * 2.f);

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

void Game::DrawDebugInfo(std::vector<Actor *> &actorsOnCamera)
{
    mSpatialHashing->Draw(mRenderer, mCameraPos, mWindowWidth, mWindowHeight);

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

    SDL_RenderSetLogicalSize(mRenderer, mRealWindowWidth, mRealWindowHeight);

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
