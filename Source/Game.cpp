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
#include "Json.h"
#include "CSV.h"
#include "Random.h"
#include "Game.h"
#include "HUD.h"
#include "SpatialHashing.h"
#include "Actors/Actor.h"
#include "Actors/Punk.h"
#include "Actors/Block.h"
#include "Actors/Spawner.h"
#include "Actors/Enemy.h"
#include "Actors/Portal.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/DrawComponents/DrawSpriteComponent.h"
#include "Components/DrawComponents/DrawPolygonComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"

Game::Game(int windowWidth, int windowHeight)
    : mWindow(nullptr), mRenderer(nullptr), mTicksCount(0), mIsRunning(true), mWindowWidth(windowWidth), mWindowHeight(windowHeight), mPunk(nullptr), mHUD(nullptr), mBackgroundColor(0, 0, 0), mModColor(255, 255, 255), mCameraPos(Vector2::Zero), mAudio(nullptr), mGameTimer(0.0f), mGameTimeLimit(0), mSceneManagerTimer(0.0f), mSceneManagerState(SceneManagerState::None), mGameScene(GameScene::MainMenu), mNextScene(GameScene::Level1), mBackgroundTexture(nullptr), mBackgroundSize(Vector2::Zero), mBackgroundPosition(Vector2::Zero)
{
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("ETER", 0, 0, mWindowWidth, mWindowHeight, 0);
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
                                         FIRST_LEVEL_WIDTH * TILE_SIZE,
                                         FIRST_LEVEL_HEIGHT * TILE_SIZE);
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    SetGameScene(GameScene::MainMenu);

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    // Scene Manager FSM: using if/else instead of switch
    if (mSceneManagerState == SceneManagerState::None)
    {
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2)
        {
            mNextScene = scene;
            mSceneManagerState = SceneManagerState::Entering;
            mSceneManagerTimer = transitionTime;
        }
        else
        {
            SDL_Log("Invalid game scene: %d", static_cast<int>(scene));
            return;
        }
    }
    else
    {
        SDL_Log("Scene Manager is already in a transition state");
        return;
    }
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

    // Reset game timer
    mGameTimer = 0.0f;

    // Reset gameplau state
    mGamePlayState = GamePlayState::Playing;

    // Reset scene manager state
    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f, FIRST_LEVEL_WIDTH * TILE_SIZE, FIRST_LEVEL_HEIGHT * TILE_SIZE);

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
    {
        // Set background color
        mBackgroundColor.Set(24.0f, 22.0f, 30.0f);

        // Set background color
        // SetBackgroundImage("../Assets/Sprites/Background.png", Vector2(TILE_SIZE,0), Vector2(6784,448));

        // Initialize main menu actors
        LoadMainMenu();
    }
    else if (mNextScene == GameScene::Level1)
    {
        // Start Music
        // mMusicHandle = mAudio->PlaySound("MusicMain.ogg", true);

        // Set background color
        // mBackgroundColor.Set(107.0f, 140.0f, 255.0f);

        // Create HUD
        // mHUD = new HUD(this, "../Assets/Fonts/SMB.ttf");

        // Reset HUD
        // mGameTimeLimit = 400;
        // mHUD->SetTime(mGameTimeLimit);
        // mHUD->SetLevelName("1-1");

        //  Set background color
        // SetBackgroundImage("../Assets/Sprites/Background.png", Vector2(TILE_SIZE,0), Vector2(6784,448));

        // Draw Flag
        //  auto flag = new Actor(this);
        // flag->SetPosition(Vector2(FIRST_LEVEL_WIDTH * TILE_SIZE - (16 * TILE_SIZE) - 16, 3 * TILE_SIZE));

        // Add a flag sprite
        // new DrawSpriteComponent(flag, "../Assets/Sprites/Background_Flag.png", 32.0f, 32.0f, 1);

        // Add a flag pole taking the entire height
        // new AABBColliderComponent(flag, 30, 0, 4, TILE_SIZE * FIRST_LEVEL_HEIGHT, ColliderLayer::Pole, true);

        // Initialize actors
        LoadLevel("../Assets/Levels/map1/map.json", FIRST_LEVEL_WIDTH, FIRST_LEVEL_HEIGHT);
    }
    else if (mNextScene == GameScene::Level2)
    {
        // Start Music
        //  mMusicHandle = mAudio->PlaySound("MusicUnderground.ogg", true);

        // Set background color
        // mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        // Set mod color
        // mModColor.Set(0.0f, 255.0f, 200.0f);

        // Create HUD
        // mHUD = new HUD(this, "../Assets/Fonts/SMB.ttf");

        // Reset HUD
        // mGameTimeLimit = 400;
        // mHUD->SetTime(mGameTimeLimit);
        // mHUD->SetLevelName("1-2");

        // Initialize actors
        LoadLevel("../Assets/Levels/map1/layer0.csv", FIRST_LEVEL_WIDTH, FIRST_LEVEL_HEIGHT);
    }

    // Set new scene
    mGameScene = mNextScene;
}

void Game::LoadMainMenu()
{
    // Load font
    UIScreen *mainMenu = new UIScreen(this, "../Assets/Fonts/SMB.ttf");

    // Draw a row of blocks on the ground
    for (int x = 0; x < FIRST_LEVEL_WIDTH; ++x)
    {
        Vector2 blockPos = Vector2(x * TILE_SIZE, (FIRST_LEVEL_HEIGHT - 2) * TILE_SIZE);
        // mainMenu->AddImage("../Assets/Sprites/Blocks/BlockA.png", blockPos, Vector2(TILE_SIZE, TILE_SIZE));
    }

    // Draw Punk
    Vector2 punkPos = Vector2(4 * TILE_SIZE, (FIRST_LEVEL_HEIGHT - 3) * TILE_SIZE);
    // mainMenu->AddImage("../Assets/Sprites/Punk/Idle.png", punkPos, Vector2(TILE_SIZE, TILE_SIZE));

    // Add title
    const Vector2 titleSize = Vector2(178.0f, 110.0f) * 2.0f;
    const Vector2 titlePos = Vector2(mWindowWidth / 2.0f - titleSize.x / 2.0f, 50.0f);
    mainMenu->AddImage("../Assets/Sprites/eter.png", titlePos, titleSize);

    // Add menu buttons
    const Vector2 buttonSize = Vector2(200.0f, 40.0f);
    const Vector2 button1Pos = Vector2(mWindowWidth / 2.0f - buttonSize.x / 2.0f, titlePos.y + titleSize.y + 30.0f);
    const Vector2 button2Pos = Vector2(mWindowWidth / 2.0f - buttonSize.x / 2.0f, button1Pos.y + buttonSize.y + 5.0f);

    mainMenu->AddButton("Play", button1Pos, buttonSize, [this]()
                        { SetGameScene(GameScene::Level1); });

    mainMenu->AddButton("Quit", button2Pos, buttonSize, [this]
                        { Quit(); });
}

void Game::LoadLevel(const std::string &levelName, const int levelWidth, const int levelHeight)
{
    std::ifstream in(levelName);
    if (!in.is_open())
    {
        SDL_Log("Could not open map file");
        return;
    }

    try {
        in >> mMapJson;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        SDL_Log("JSON parse error: %s", e.what());
        return;
    }

    // Instantiate level actors
    BuildLevel("../Assets/Levels/map1/blocks/");
    mPunk = new Punk(this, 1000.0f, -1000.0f);
    mPunk->SetPosition(Vector2(100.0f, 100.0f));

    mEnemy = new Enemy(this, mPunk);
    mEnemy->SetPosition(Vector2(500.0f, 300.0f));
    mEnemy->Start();

    const auto &portal = new Portal(this);
    portal->SetPosition(Vector2(1160.0f, 32.0f));
}

static std::string ZeroPadId(int id, int width = 3)
{
    std::ostringstream oss;
    oss << std::setw(width) << std::setfill('0') << id;
    return oss.str();
}

void Game::BuildLevel(std::string blocksDir)
{
    const auto &layers = mMapJson.at("layers");

    int layer_idx = 2;
    for (const auto &layer : layers) {
        const auto &tiles = layer.at("tiles");

        for (const auto &t : tiles)
        {
            int id = std::stoi(t.at("id").get<std::string>());
            int x = t.at("x").get<int>();
            int y = t.at("y").get<int>();

            Vector2 position(x * TILE_SIZE, y * TILE_SIZE);

            std::string texPath = blocksDir + "/" + ZeroPadId(id) + ".png";

            auto *block = new Block(this,
                                    texPath,
                                    Layers[layer_idx]);

            block->SetPosition(position);
        }

        layer_idx--;
    }
}

int **Game::ReadLevelData(const std::string &fileName, int width, int height)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Failed to open level file: \'%s\'", fileName.c_str());
        return nullptr;
    }

    int **levelData = new int *[height];
    for (int i = 0; i < height; i++)
    {
        levelData[i] = new int[width];
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line))
    {
        if (row >= height)
        {
            SDL_Log("Level file has more rows than expected height %d", height);
            break;
        }

        std::vector<int> tokens = CSVHelper::Split(line);
        if (tokens.size() != width)
        {
            SDL_Log("Level file has a problem in the width");
            return nullptr;
        }

        for (int col = 0; col < width; col++)
        {
            levelData[row][col] = tokens[col];
        }
        row++;
    }

    file.close();
    return levelData;
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
        }
    }

    ProcessInputActors();
}

void Game::ProcessInputActors()
{
    if (mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor *> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos, mWindowWidth, mWindowHeight);

        const Uint8 *state = SDL_GetKeyboardState(nullptr);

        bool isPunkoOnCamera = false;
        for (auto actor : actorsOnCamera)
        {
            actor->ProcessInput(state);

            if (actor == mPunk)
            {
                isPunkoOnCamera = true;
            }
        }

        // If Punk is not on camera, process input for him
        if (!isPunkoOnCamera && mPunk)
        {
            mPunk->ProcessInput(state);
        }
    }
}

void Game::HandleKeyPressActors(const int key, const bool isPressed)
{
    if (mGamePlayState == GamePlayState::Playing)
    {
        // Get actors on camera
        std::vector<Actor *> actorsOnCamera =
            mSpatialHashing->QueryOnCamera(mCameraPos, mWindowWidth, mWindowHeight);

        // Handle key press for actors
        bool isPunkoOnCamera = false;
        for (auto actor : actorsOnCamera)
        {
            actor->HandleKeyPress(key, isPressed);

            if (actor == mPunk)
            {
                isPunkoOnCamera = true;
            }
        }

        // If Punk is not on camera, handle key press for him
        if (!isPunkoOnCamera && mPunk)
        {
            mPunk->HandleKeyPress(key, isPressed);
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
            mAudio->PlaySound("Coin.wav");
            mAudio->PauseSound(mMusicHandle);
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;
            mAudio->PlaySound("Coin.wav");
            mAudio->ResumeSound(mMusicHandle);
        }
    }
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
        ;

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    if (mGamePlayState != GamePlayState::Paused && mGamePlayState != GamePlayState::GameOver)
    {
        // Reinsert all actors and pending actors
        UpdateActors(deltaTime);
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

    // ---------------------
    // Game Specific Updates
    // ---------------------
    if (mGameScene != GameScene::MainMenu && mGamePlayState == GamePlayState::Playing)
    {
        // Reinsert level time
        // UpdateLevelTime(deltaTime);
    }

    UpdateSceneManager(deltaTime);
    UpdateCamera();
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

void Game::UpdateLevelTime(float deltaTime)
{
    // Reinsert game timer
    mGameTimer += deltaTime;
    if (mGameTimer >= 0.5f)
    {
        mGameTimer = 0.0f;
        mGameTimeLimit--;

        if (mGameTimeLimit > 0)
        {
            mHUD->SetTime(mGameTimeLimit);
        }
        else
        {
            // Kill Punk if time limit is reached
            mHUD->SetTime(mGameTimeLimit);
            mPunk->Kill();
        }
    }
}

void Game::UpdateCamera()
{
    if (mPunk)
    {
        float cameraX = mPunk->GetPosition().x - mWindowWidth / 2.0f;
        float cameraY = mPunk->GetPosition().y - mWindowHeight / 2.0f;
        float maxCameraX = FIRST_LEVEL_WIDTH * TILE_SIZE - mWindowWidth;
        float maxCameraY = FIRST_LEVEL_HEIGHT * TILE_SIZE - mWindowWidth;

        mCameraPos.x = std::min(cameraX, maxCameraX);
        mCameraPos.y = std::min(cameraY, maxCameraY);

        mCameraPos.x = std::max(0.0f, mCameraPos.x);
        mCameraPos.y = std::max(0.0f, mCameraPos.y);
    }
    // if (!mPunk) return;
    //
    // float horizontalCameraPos = mPunk->GetPosition().x - (mWindowWidth / 2.0f);
    //
    // if (horizontalCameraPos > mCameraPos.x)
    // {
    //     // Limit camera to the right side of the level
    //     float maxCameraPos = (FIRST_LEVEL_WIDTH * TILE_SIZE) - mWindowWidth;
    //     horizontalCameraPos = Math::Clamp(horizontalCameraPos, 0.0f, maxCameraPos);
    //
    //     mCameraPos.x = horizontalCameraPos;
    // }
}

void Game::UpdateActors(float deltaTime)
{
    // Get actors on camera
    std::vector<Actor *> actorsOnCamera =
        mSpatialHashing->QueryOnCamera(mCameraPos, mWindowWidth, mWindowHeight);

    bool isPunkOnCamera = false;
    for (auto actor : actorsOnCamera)
    {
        actor->Update(deltaTime);
        if (actor == mPunk)
        {
            isPunkOnCamera = true;
        }
    }

    // If Punk is not on camera, update him (player should always be updated)
    if (!isPunkOnCamera && mPunk)
    {
        mPunk->Update(deltaTime);
    }

    for (auto actor : actorsOnCamera)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            delete actor;
            if (actor == mPunk)
            {
                mPunk = nullptr;
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
        SDL_Rect dstRect = {static_cast<int>(mBackgroundPosition.x - mCameraPos.x),
                            static_cast<int>(mBackgroundPosition.y - mCameraPos.y),
                            static_cast<int>(mBackgroundSize.x),
                            static_cast<int>(mBackgroundSize.y)};

        SDL_RenderCopy(mRenderer, mBackgroundTexture, nullptr, &dstRect);
    }

    // Get actors on camera
    std::vector<Actor *> actorsOnCamera =
        mSpatialHashing->QueryOnCamera(mCameraPos, mWindowWidth, mWindowHeight);

    // Get list of drawables in draw order
    std::vector<DrawComponent *> drawables;

    for (auto actor : actorsOnCamera)
    {
        auto drawable = actor->GetComponent<DrawComponent>();
        if (drawable && drawable->IsVisible())
        {
            drawables.emplace_back(drawable);
        }
    }

    // Sort drawables by draw order
    std::sort(drawables.begin(), drawables.end(),
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

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

void Game::SetBackgroundImage(const std::string &texturePath, const Vector2 &position, const Vector2 &size)
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

    // Set background position
    mBackgroundPosition.Set(position.x, position.y);

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
