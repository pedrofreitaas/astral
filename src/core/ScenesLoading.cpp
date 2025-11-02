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
#include "../actors/enemies/Zod.h"
#include "../actors/enemies/Sith.h"
#include "../actors/Portal.h"

void Game::LoadMainMenu()
{
    UIScreen *mainMenu = new UIScreen(this, FONT_PATH_SMB);

    mainMenu->AddBackground(
        "../assets/Sprites/Menu/background.png",
        Vector2(0, 0),
        Vector2(mWindowWidth, mWindowHeight));

    Vector2 titleSize = Vector2(255.f, 92.f);
    mainMenu->AddImage(
        "../assets/Sprites/Menu/title.png",
        Vector2(mWindowWidth / 2.0f - titleSize.x / 2.0f, mWindowHeight * .1f),
        titleSize);

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
        { SetGameScene(GameScene::Bedroom); },
        Vector2(CHAR_WIDTH * 4, WORD_HEIGHT));

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);
}

void Game::LoadBedroom()
{
    mHUD = new HUD(this, FONT_PATH_INTER);

    SetMap("bedroom.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    mZoe = new Zoe(this, 1500.0f);
    mZoe->SetPosition(Vector2(32.0f, 80.0f));

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue = {
        "Hoje e dia de visitar a Vovo, Papai e Mamae ja devem estar se arrumando.",
        "Acho que eu deveria ver como eles estao..."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("leave_bedroom",
                std::move(steps),
                [this]() {});

    steps.clear();

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    dialogue = {
        "Sera que eles estao bravos porque acordei tarde?"};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));

    AddCutscene("enterBedroomPortal",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::BedroomPortal);
                });
}

void Game::LoadBedroomPortal()
{
    mHUD = new HUD(this, FONT_PATH_INTER);

    SetMap("bedroomPortal.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    mZoe = new Zoe(this, 1500.0f);
    mZoe->SetPosition(Vector2(10.0f, 32.0f));

    auto portal = new Portal(this, Vector2(0.f, 72.f) + mMap->GetCenter());

    std::vector<std::unique_ptr<Step>> steps;
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetZoe()->GetCenter() + Vector2(-10.f, 0.f); },
        20.f));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetZoe()->GetCenter() + Vector2(10.f, 0.f); },
        20.f));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    std::vector<std::string> dialogue = {
        "Pai? Mae? Cade voces?",
        "O que e isso no meio do quarto?",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this, portal]()
        { return portal->GetCenter(); },
        120.0f,
        true));
    steps.push_back(std::make_unique<UnspawnStep>(
        this, 
        [this]()
        { return this->GetZoe(); }));

    AddCutscene("portalSuck",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::Level1);
                });
}

void Game::LoadFirstLevel()
{
    mHUD = new HUD(this, Game::FONT_PATH_INTER);

    SetApplyGravityScene(true);

    SetMap("level1.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/galaxy.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    Enemy* en = new Zod(this, 1500.0f, Vector2(600.0f, mMap->GetHeight() - 80.0f));
    mEnemies.push_back(en);

    en = new Sith(this, 1200.0f, Vector2(200.0f, mMap->GetHeight() - 320.0f));
    mEnemies.push_back(en);

    mZoe = new Zoe(this, 1500.0f);
    mZoe->SetPosition(Vector2(32.0f, mMap->GetHeight() - 80.0f));

    std::vector<std::unique_ptr<Step>> steps;
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        {
            return Vector2(GetZoe()->GetCenter().x+64.f, GetZoe()->GetCenter().y);
        },
        100.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        {
            return Vector2(GetZoe()->GetCenter().x-2.f, GetZoe()->GetCenter().y);
        },
        20.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        {
            return Vector2(GetZoe()->GetCenter().x+4.f, GetZoe()->GetCenter().y);
        },
        20.0f));
    steps.push_back(std::make_unique<WaitStep>(this, 1.5f));

    std::vector<std::string> dialogue = {
        "O que... o que e isso?",
        "Onde estou? Parece que estou no espaco.",
        "Mas algo esta errado, eu sou uma pessoa comum, eu nao deveria estar aqui."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(0.0f, mMap->GetHeight() - mWindowHeight / 2.0f)));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetStar(); },
        [this]()
        { return Vector2(mWindowWidth / 2.0f, mMap->GetHeight() - mWindowHeight / 2.0f); },
        250.0f));

    dialogue = {
        "Uma estrela? Ela parece estar indo para algum lugar?",
        "Nao tenho outra opcao, tenho que segui-la."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetStar(); },
        [this]()
        { return Vector2(mWindowWidth * 1.5f, mMap->GetHeight() - mWindowHeight * 1.2f); },
        320.0f));

    dialogue = {
        "Espere! Volte aqui!",
        "Onde sera que ela foi? Preciso saber se ela esta me levando para algum lugar..."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    AddCutscene("Intro",
                std::move(steps),
                [this]() {});

    StartCutscene("Intro");
}
