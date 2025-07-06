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
#include "Actors/Item.h"
#include "UIElements/UIScreen.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/DrawComponents/DrawSpriteComponent.h"
#include "Components/DrawComponents/DrawPolygonComponent.h"
#include "Components/ColliderComponents/AABBColliderComponent.h"
#include <filesystem>
#include <regex>
#include "UIElements/DialogueSystem.h"


namespace fs = std::filesystem;

Game::Game(int windowWidth, int windowHeight)
    : mWindow(nullptr), mRenderer(nullptr), mTicksCount(0), mIsRunning(true),
      mWindowWidth(windowWidth), mWindowHeight(windowHeight), mPunk(nullptr), mHUD(nullptr), mBackgroundColor(0, 0, 0), mModColor(255, 255, 255), mCameraPos(Vector2::Zero), mAudio(nullptr), mGameTimer(0.0f), mGameTimeLimit(0), mSceneManagerTimer(0.0f), mSceneManagerState(SceneManagerState::None), mGameScene(GameScene::MainMenu), mNextScene(GameScene::Level1), mBackgroundTexture(nullptr), mBackgroundSize(Vector2::Zero), mBackgroundPosition(Vector2::Zero)
{
}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("ETER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWindowWidth, mWindowHeight, 0);
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
                                         LEVEL_WIDTH * TILE_SIZE,
                                         LEVEL_HEIGHT * TILE_SIZE);
    DialogueSystem::Get()->Initialize(this);
    mTicksCount = SDL_GetTicks();

    // Init all game actors
    SetGameScene(GameScene::MainMenu);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    return true;
}

void Game::SetGameScene(Game::GameScene scene, float transitionTime)
{
    // Scene Manager FSM: using if/else instead of switch
    if (mSceneManagerState == SceneManagerState::None)
    {
        //if (scene == GameScene::MainMenu || scene == GameScene::Intro || scene == GameScene::Level1 || scene == GameScene::Level2 || scene == GameScene::FinalScene)
        if (scene == GameScene::MainMenu || scene == GameScene::Level1 || scene == GameScene::Level2 || scene == GameScene::Ending_Stay || scene == GameScene::Ending_GoHome)

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
    mSpatialHashing = new SpatialHashing(TILE_SIZE * 4.0f, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE);

    // Scene Manager FSM: using if/else instead of switch
    if (mNextScene == GameScene::MainMenu)
    {
        // Set background color
        mBackgroundColor.Set(24.0f, 22.0f, 30.0f);

        // Initialize main menu actors
        LoadMainMenu();
    }
    // else if (mNextScene == GameScene::Intro) {
    //     mMusicHandle = mAudio->PlaySound("MainTheme.ogg", true);
    //     mBackgroundColor.Set(0.0f, 0.0f, 0.0f);
    //
    //     UIScreen *intro = new UIScreen(this, "../Assets/Fonts/SMB.ttf");
    //     const Vector2 titleSize = Vector2(mWindowWidth, mWindowHeight);
    //     const Vector2 titlePos = Vector2(0.0f, 0.0f);
    //     intro->AddImage("../Assets/Sprites/img.png", titlePos, titleSize);
    //
    //     const Vector2 buttonSize = Vector2(200.0f, 40.0f);
    //     const Vector2 button1Pos = Vector2(mWindowWidth / 2.0f - buttonSize.x / 2.0f, mWindowHeight * 0.8f);
    //     intro->AddButton("Continue", button1Pos, buttonSize, [this]()
    //                 { SetGameScene(GameScene::Level1);});
    // }
    else if (mNextScene == GameScene::Level1)
    {
        // Start Music
        mMusicHandle = mAudio->PlaySound("MainTheme.ogg", true);

        // Set background color
        mBackgroundColor.Set(107.0f, 140.0f, 255.0f);
        mHUD = new HUD(this, "../Assets/Fonts/VT323-Regular.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);

        LoadLevel("../Assets/Levels/map_1/map_tiled.json", "../Assets/Levels/map_1/");

        mPunk = new Punk(this, 1000.0f, -1000.0f);
        mPunk->SetPosition(Vector2(128.0f, 1088.0f));

        auto spawner = new Spawner(this, 3000.f, 0);
        spawner->SetPosition(Vector2(500.0f, 1000.0f));

       // const auto &portal = new Portal(this);
       // portal->SetPosition(Vector2(622.0f, 210.0f));
        DialogueSystem::Get()->StartDialogue(
        { // Um vetor com as falas
            "Punk: Ugh... Minha cabeca... Onde estou?",
            "Punk: A ultima coisa que lembro... foi de um clarao.",
            "Punk: Tenho que sair desta floresta. E descobrir o que esta acontecendo."

        },
        [this]() { // Esta função será chamada quando o diálogo terminar
            // Retorna o estado do jogo para "Playing" para que a fase comece.
            SetGamePlayState(GamePlayState::Playing);
        }
    );
        const auto &key = new Item(
            this,
            "../Assets/Levels/map_2/blocks/dungeon_3/021.png",
            [this](Item&){ mPunk->FindKey(); },
            10, 10
        );
        key->SetPosition(Vector2(1510.0f, 125.0f));

        const auto &heart1 = new Item(
    this,
    "../Assets/Sprites/Itens/07.png",
    [this](Item&){ mPunk->FindHeart(); },
    10, 10
);
        heart1->SetPosition(Vector2(1749.0f, 392.0f));

        const auto &heart2 = new Item(
    this,
    "../Assets/Sprites/Itens/07.png",
    [this](Item&){ mPunk->FindHeart(); },
    10, 10
);
        heart2->SetPosition(Vector2(484.0f, 399.0f));


    }
    else if (mNextScene == GameScene::Level2)
    {
        // Start Music
        mAudio->StopSound(mMusicHandle);
        mMusicHandle = mAudio->PlaySound("BattleTheme.mp3", true);

        mHUD = new HUD(this, "../Assets/Fonts/VT323-Regular.ttf");

        mGameTimeLimit = 400;
        mHUD->SetTime(mGameTimeLimit);

        // Set background color
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        // Settings actors
        mPunk = new Punk(this, 1000.0f, -1000.0f);
        mPunk->SetPosition(Vector2(1138.0f, 224.0f));

        auto spawner = new Spawner(this, 3000.0f, 1);
        spawner->SetPosition(Vector2(500.0f, 100.0f));

        // Initialize actors
        LoadLevel("../Assets/Levels/map_2/map_tiled.json", "../Assets/Levels/map_2/");

        const auto &key = new Item(
            this,
            "../Assets/Levels/map_2/blocks/dungeon_3/021.png",
            [this](Item&){ mPunk->FindKey(); },
            10, 10
        );
        key->SetPosition(Vector2(160.0f, 192.0f));

        DialogueSystem::Get()->StartDialogue(
{
    "Punk: Oque? Parece que agora estou em uma dimensao totalmente diferente",
    "Punk: Sera que este e o ETER? Como o mestre havia me falado?"
},
[this]() {
    SetGamePlayState(GamePlayState::Playing);
}
);
            const auto &heart1 = new Item(
    this,
    "../Assets/Sprites/Itens/07.png",
    [this](Item&){ mPunk->FindHeart(); },
    10, 10
    );
            heart1->SetPosition(Vector2(367.0f, 364.0f));
        }

    else if (mNextScene == GameScene::Ending_Stay) {
        mAudio->StopSound(mMusicHandle);
        mMusicHandle = mAudio->PlaySound("BattleTheme.mp3", true);
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        UIScreen *final = new UIScreen(this, "../Assets/Fonts/VT323-Regular.ttf");
        const Vector2 titleSize = Vector2(mWindowWidth, mWindowHeight);
        const Vector2 titlePos = Vector2(0.0f, 0.0f);
        final->AddImage("../Assets/Sprites/Final_eter.png", titlePos, titleSize);

        DialogueSystem::Get()->StartDialogue(
{
    "Voz: O sacrificio foi feito.",
    "Voz: Voce se tornou o guardiao do eter"
},
[this]() { Quit(); });

    }

    else if (mNextScene == GameScene::Ending_GoHome) {
        // Start Music
        mAudio->StopSound(mMusicHandle);
        mMusicHandle = mAudio->PlaySound("BattleTheme.mp3", true);
        mBackgroundColor.Set(0.0f, 0.0f, 0.0f);

        UIScreen *final = new UIScreen(this, "../Assets/Fonts/VT323-Regular.ttf");
        const Vector2 titleSize = Vector2(mWindowWidth, mWindowHeight);
        const Vector2 titlePos = Vector2(0.0f, 0.0f);
        final->AddImage("../Assets/Sprites/Final_casa1.png", titlePos, titleSize);

        DialogueSystem::Get()->StartDialogue(
    {
        "Voz: Voce voltou pra casa.",
        "Voz: O Eter foi deixado para tras."
    },
    [this]() { Quit(); });
    }

    // Set new scene
    mGameScene = mNextScene;
}

void Game::LoadMainMenu()
{
    int CHAR_WIDTH = 16;
    int WORD_HEIGHT = 20;

    UIScreen *mainMenu = new UIScreen(this, "../Assets/Fonts/VT323-Regular.ttf");    
    mainMenu->AddBackground("../Assets/Sprites/Menu/eter_extended_side.jpg", Vector2(0,0), Vector2(mWindowWidth, mWindowHeight));

    const Vector2 titleSize = Vector2(300.0f, 180.0f);
    const Vector2 titlePos = Vector2(mWindowWidth / 3.1f - titleSize.x / 2.0f, 20.0f);
    mainMenu->AddImage("../Assets/Sprites/Menu/eter_title.png", titlePos, titleSize);
    
    const Vector2 buttonSize = Vector2(230.0f, 55.0f);
    const Vector2 button1Pos = Vector2(mWindowWidth / 4.0f * 3 - buttonSize.x / 2.0f, titlePos.y + titleSize.y + 15.0f);
    const Vector2 button2Pos = Vector2(mWindowWidth / 4.0f * 3 - buttonSize.x / 2.0f, button1Pos.y + buttonSize.y + 5.0f);
    const Vector2 button3Pos = Vector2(mWindowWidth / 4.0f * 3 - buttonSize.x / 2.0f, button2Pos.y + buttonSize.y + 5.0f);

    mainMenu->AddButton(" ", button1Pos, buttonSize, [this]()
                        { SetGameScene(GameScene::Level1); });
    mainMenu->AddText("Play", button1Pos+Vector2(buttonSize.x/2 - CHAR_WIDTH*2, buttonSize.y/3), Vector2(CHAR_WIDTH * 4, WORD_HEIGHT), 40, 1024, Color::White);
    mainMenu->AddImage("../Assets/Sprites/Menu/button.png", button1Pos, buttonSize);

    mainMenu->AddButton(" ", button2Pos, buttonSize, [this]()
    {
        int CHAR_WIDTH = 14;
        int WORD_HEIGHT = 20;

        UIScreen* howToPlay = new UIScreen(this, "../Assets/Fonts/VT323-Regular.ttf");
        const Vector2 howToPlaySize = Vector2(504.0f, 365.0f);
        const Vector2 howToPlayPos = Vector2(mWindowWidth/2 - howToPlaySize.x/2 + 5, 20.0f);
        howToPlay->AddBackground("../Assets/Sprites/Menu/info_frame.png", howToPlayPos, howToPlaySize);

        // Texto de instrução
        std::vector<std::string> instructionsList = {
            "Use W A S D para se mover",
            "Use o mouse para mirar",
            "Clique para atirar",
            "Evite os inimigos!",
            "Aperte F para trocar de arma",
            "Seus tiros recarregam,",
            "enquanto não estiver atirando",
        };
        for(int i = 0; i < instructionsList.size(); ++i)
        {
            howToPlay->AddText(instructionsList[i], Vector2(mWindowWidth/2 - CHAR_WIDTH * instructionsList[i].size()/2, howToPlayPos.y + howToPlaySize.y / 3 + WORD_HEIGHT * i),
                               Vector2(CHAR_WIDTH * instructionsList[i].size(), WORD_HEIGHT), 36, 1024, Color::White);
        }

        // Botão OK
        Vector2 okSize = Vector2(150, 50);
        Vector2 okPos = Vector2(mWindowWidth * 0.39, mWindowHeight * 0.72);
        howToPlay->AddButton(" ", okPos, okSize, [this]()
        {
            PopUI(); // Sai do How To Play
        });
        howToPlay->AddText("OK", Vector2(mWindowWidth * 0.475, mWindowHeight * 0.75), Vector2(CHAR_WIDTH*2, WORD_HEIGHT), 40, 1024, Color::White);
        howToPlay->AddImage("../Assets/Sprites/Menu/button.png", okPos, okSize);

        SDL_Event dummy;
        while (SDL_PeepEvents(&dummy, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN) > 0);


        PushUI(howToPlay); // Mostra tela por cima
    });
    mainMenu->AddText("How To Play", button2Pos+Vector2(buttonSize.x/2 - CHAR_WIDTH*5.5, buttonSize.y/3), Vector2(CHAR_WIDTH * 11, WORD_HEIGHT),40, 1024, Color::White);
    mainMenu->AddImage("../Assets/Sprites/Menu/button.png", button2Pos, buttonSize);

    mainMenu->AddButton(" ", button3Pos, buttonSize, [this]
                        { Quit(); });
    mainMenu->AddText("Quit", button3Pos+Vector2(buttonSize.x/2 - CHAR_WIDTH*2, buttonSize.y/3), Vector2(CHAR_WIDTH * 4, WORD_HEIGHT), 40, 1024, Color::White);
    mainMenu->AddImage("../Assets/Sprites/Menu/button.png", button3Pos, buttonSize);
}

void Game::LoadLostScreen()
{
    // Cria uma nova tela de UI para a derrota
    UIScreen* lostScreen = new UIScreen(this, "../Assets/Fonts/SMB.ttf");

    // Adiciona o texto "Voce Perdeu!"
    const Vector2 titleSize = Vector2(400.0f, 100.0f);
    const Vector2 titlePos = Vector2(mWindowWidth / 2.0f - titleSize.x / 2.0f, 150.0f);
    lostScreen->AddText("Voce Perdeu!", titlePos, titleSize, 40, 1024, Color::Red);

    // Adiciona botões com opções para o jogador
    const Vector2 buttonSize = Vector2(250.0f, 50.0f);
    const Vector2 retryPos = Vector2(mWindowWidth / 2.0f - buttonSize.x / 2.0f, 300.0f);
    const Vector2 menuPos = Vector2(mWindowWidth / 2.0f - buttonSize.x / 2.0f, retryPos.y + buttonSize.y + 15.0f);

    // Botão "Tentar Novamente" que reseta a cena atual
    lostScreen->AddButton("Tentar Novamente", retryPos, buttonSize, [this]() {
        ResetGameScene(.25f); // Função que você já tem para reiniciar a fase
    });

    // Botão "Voltar ao Menu" que leva para a cena do menu principal
    // Botão "Voltar ao Menu"
    lostScreen->AddButton("Voltar ao Menu", menuPos, buttonSize, [this]() {
        SetGameScene(GameScene::MainMenu, .25f);
    });
}

void Game::LoadLevel(const std::string &levelPath, const std::string &blocksDir)
{
    std::ifstream in(levelPath);
    if (!in.is_open())
    {
        SDL_Log("Could not open map file");
        return;
    }

    try
    {
        in >> mMapJson;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        SDL_Log("JSON parse error: %s", e.what());
        return;
    }

    // Instantiate level actors
    BuildLevel(blocksDir);
}


std::unordered_map<int, std::string> LoadGlobalTileMap(
    const nlohmann::json &mapJson,
    const std::string &mapDir)
{
    std::unordered_map<int, std::string> globalTileMap;

    std::regex tileRegex("<tile id=\"(\\d+)\">");
    std::regex imageRegex("source=\"([^\"]+)\"");

    for (const auto &tileset : mapJson["tilesets"])
    {
        int firstGid = tileset["firstgid"];
        std::string tsxRelPath = tileset["source"];  // ex: "../blocks/tileset_1.tsx"

        fs::path tsxPath = fs::path(mapDir) / tsxRelPath;
        fs::path tilesetDir = tsxPath.parent_path();

        std::ifstream file(tsxPath);
        if (!file.is_open())
        {
            SDL_Log("Erro ao abrir TSX: %s", tsxPath.string().c_str());
            continue;
        }

        std::string line;
        int currentLocalId = -1;

        while (std::getline(file, line))
        {
            std::smatch match;

            if (std::regex_search(line, match, tileRegex))
            {
                currentLocalId = std::stoi(match[1].str());
            }
            else if (currentLocalId != -1 && std::regex_search(line, match, imageRegex))
            {
                std::string imageFile = match[1].str();
                int globalId = firstGid + currentLocalId;
                std::string fullPath = (tilesetDir / imageFile).string();

                globalTileMap[globalId] = fullPath;

                currentLocalId = -1;
            }
        }
    }

    return globalTileMap;
}

void Game::BuildLevel(std::string blocksDir)
{
    auto globalTileMap = LoadGlobalTileMap(mMapJson, blocksDir);

    const auto &layers = mMapJson.at("layers");

    for (const auto &layer : layers)
    {
        const auto &tiles = layer.at("data");
        int width = layer.at("width");
        int height = layer.at("height");
        std::string name = layer.at("name").get<std::string>();

        int layer_idx;
        if (name.find("ground") != std::string::npos)
            layer_idx = 0;
        else if (name.find("player") != std::string::npos)
            layer_idx = 1;
        else if (name.find("details_top") != std::string::npos)
            layer_idx = 2;
        else if (name.find("details_down") != std::string::npos)
            layer_idx = 3;
        else
            layer_idx = 4;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = y * width + x;
                int gid = tiles[index];

                if (gid == 0) continue; // tile vazio

                auto it = globalTileMap.find(gid);
                if (it == globalTileMap.end()) continue;

                std::string texPath = it->second;
                Vector2 position(x * TILE_SIZE, y * TILE_SIZE);

                auto *block = new Block(this, texPath, Layers[layer_idx]);
                block->SetPosition(position);
            }
        }
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

    const Uint8 *keyState = SDL_GetKeyboardState(nullptr);
    if (DialogueSystem::Get()->IsActive())
    {
        DialogueSystem::Get()->HandleInput(keyState);
    }
    // else if (mGamePlayState == GamePlayState::GameOver)
    // {
    //     // Não faz nada, efetivamente pausando o input do jogador.
    // }
    else {
        ProcessInputActors();
    }
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
            //mAudio->PlaySound("Coin.wav");
            mAudio->PauseSound(mMusicHandle);
        }
        else if (mGamePlayState == GamePlayState::Paused)
        {
            mGamePlayState = GamePlayState::Playing;
            //mAudio->PlaySound("Coin.wav");
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

    if (mGamePlayState == GamePlayState::Playing)
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
    if (mGameScene != GameScene::MainMenu)
    {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        mHUD->UpdateMousePosition(mouseX, mouseY);
        
        if(mGamePlayState == GamePlayState::Playing){
            // Reinsert level time
            UpdateLevelTime(deltaTime);
            if (mPunk)
            {
                mHUD->UpdateLives(mPunk->Lives());
                mHUD->UpdateAmmo(mPunk->GetAmmo(), mPunk->GetMaxAmmo());
                mHUD->UpdateGun(mPunk->GetCurrentWeaponName());
            }
        }   
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
    if (mGamePlayState != GamePlayState::Playing)
    {
        return;
    }

    mGameTimer += deltaTime;

    while (mGameTimer >= 1.0f)
    {
        // Subtraímos 1.0f em vez de zerar o timer. Isso mantém a precisão.
        mGameTimer -= 1.0f;
        mGameTimeLimit--;

        mHUD->SetTime(std::max(0, mGameTimeLimit));
        if (mGameTimeLimit <= 0)
        {
            mPunk->Kill();
            SetGamePlayState(GamePlayState::GameOver);
            //LoadLostScreen();
            break;
        }
    }
}

void Game::UpdateCamera()
{
    if (mPunk)
    {
        float cameraX = mPunk->GetPosition().x - mWindowWidth / 2.0f;
        float cameraY = mPunk->GetPosition().y - mWindowHeight / 2.0f;
        float maxCameraX = LEVEL_WIDTH * TILE_SIZE - mWindowWidth;
        float maxCameraY = LEVEL_HEIGHT * TILE_SIZE - mWindowWidth;

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
    //     float maxCameraPos = (LEVEL_WIDTH * TILE_SIZE) - mWindowWidth;
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
        std::vector<DrawComponent*> actorDrawables = actor->GetComponents<DrawComponent>();

        for (auto drawable : actorDrawables)
        {
            if (drawable && drawable->IsVisible())
            {
                drawables.emplace_back(drawable);
            }
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
    DialogueSystem::Get()->Draw(mRenderer);

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
