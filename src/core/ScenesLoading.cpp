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
#include "../actors/Portal.h"

void Game::LoadMainMenu()
{
    UIScreen *mainMenu = new UIScreen(this, FONT_PATH_SMB);
    isEnding = false;

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
        49.f,
        19.f);

    const Vector2 playButtonPos = Vector2(
        mWindowWidth / 2.0f - playButtonSize.x / 2.0f,
        mWindowHeight*.6f);

    mainMenu->AddTransparentButton(
        playButtonPos,
        playButtonSize,
        // [this]() { SetGameScene(GameScene::Bedroom); });
        [this]() { SetGameScene(GameScene::Tests); });
    
    mainMenu->AddImage(
        "../assets/Sprites/Menu/playButton.png",
        playButtonPos,
        playButtonSize);

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);

    mAudio->PlaySound("mainMenuTheme.ogg", true);
}

void Game::LoadBedroom()
{
    // mHUD = new HUD(this, FONT_PATH_INTER);

    SetMap("bedroom.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue = {
        "Hoje e dia de visitar a Vovo, Papai e Mamae ja devem estar se arrumando.",
        "Acho que eu deveria ver como eles estao..."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("leave_bedroom",
                std::move(steps),
                [this]() {});

    steps.clear();

    dialogue = {
        "Sera que eles estao bravos porque acordei tarde?"};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("enterBedroomPortal",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::BedroomPortal);
                });

    mZoe->SetAbilitiesLocked(true);

    mAudio->PlaySound("bedroomTheme.ogg", true);
}

void Game::LoadBedroomPortal()
{
    // mHUD = new HUD(this, FONT_PATH_INTER);

    SetMap("bedroomPortal.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    auto portal = new Portal(this, Vector2(0.f, 72.f) + mMap->GetCenter());

    std::vector<std::unique_ptr<Step>> steps;
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetZoe()->GetCenter() + Vector2(-10.f, 0.f); },
        20.f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetZoe()->GetCenter() + Vector2(10.f, 0.f); },
        20.f));
    std::vector<std::string> dialogue = {
        "Pai? Mae? Cade voces?",
        "O que e isso no meio do quarto?",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    steps.push_back(std::make_unique<SoundStep>(this, "portalSuck.wav"));
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
    
    mZoe->SetAbilitiesLocked(true);
    mAudio->PlaySound("bedroomTheme.ogg", true);
}

void Game::LoadFirstLevel()
{
    mHUD = new HUD(this, Game::FONT_PATH_INTER);

    SetApplyGravityScene(true);
    SetCameraCenterToLogicalWindowSizeCenter();

    SetMap("level1.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/galaxy.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue;

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x + 64.f, GetZoe()->GetCenter().y);}));

    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x - 2.f, GetZoe()->GetCenter().y);}));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x + 4.f, GetZoe()->GetCenter().y);}));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    dialogue = {
        "O que... o que e isso?",
        "Onde estou? Parece que estou no espaco.",
        "Mas algo esta errado, eu sou uma pessoa comum, eu nao deveria estar aqui."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(30.f, 560.f)));
    
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(330.f, 530.f); }));

    dialogue = {
        "Uma estrela? Ela parece estar indo para algum lugar?",
        "Nao tenho outra opcao, tenho que segui-la."};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(730.f, 330.f); }));

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

    steps.clear();

    dialogue = {
        "Oi Zoe! Seja bem vinda ao Espaco Astral!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    dialogue = {
        "Como assim? O que e esse lugar? Quem e voce?",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Se acalme garota, por enquanto nao ha muito a fazer a nao ser seguir a estrela que voce acabou de ver.",
        "Ela e a chave para voce descobrir o que esta fazendo aqui.",
        "Voce pode apertar 'A' para saltar as plataformas e chegar la em cima.",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("introduction",
                std::move(steps),
                [this](){
                    this->GetZoe()->SetAbilitiesLocked(false);
                });

    steps.clear();

    dialogue = {
        "Muito bem, alem de pular, voce tambem pode usar um impulso para se mover mais rapido.",
        "Aperte 'RB' enquanto esta no ar, se quiser ver como funciona.",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("jumped_first_platform",
                std::move(steps),
                [this](){});

    steps.clear();
    dialogue = {
        "Voce tambem pode atacar inimigos com golpes corpo a corpo, apertando 'X'.",
        "Voce pode se desviar apertando 'B' no momento correto para evitar danos.",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("jumped_second_platform",
                std::move(steps),
                [this](){});
    
    steps.clear();
    dialogue = {
        "Por fim mas nao menos importante, voce consegue lancar bolas de fogo magicas.",
        "Aperte 'Y' para lancar uma bola de fogo.",
        "Elas tem um tempo de carregamento ate poderem ser usadas novamente.",
        "Ah, eu quase esqueci, essas bolas de fogo refletem em superficies solidas, e nao causam dano a voce.",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("jumped_third_platform",
                std::move(steps),
                [this](){});
    
    steps.clear();
    dialogue = {
        "Parabens Zoe! Voce pegou bem o basico.",
        "Continue seguindo a estrela para descobrir mais sobre esse lugar misterioso.",
        "A partir de agora a minha ajuda nao serve de muito mais, entao boa sorte! Voce vai precisar..."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("exit_tutorial",
                std::move(steps),
                [this](){});

    steps.clear();
    dialogue = {
        "Ha? O que e aquilo?",
        "O que quer que seja nao me parece amigavel..."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("see_sith",
                std::move(steps),
                [this](){});

    steps.clear();
    dialogue = {
        "Parece que esses desafios foram muito faceis para voce!",
        "Acho que voce vai se sair muito bem nos proximos!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    dialogue = {
        "E isso jogador! Essa foi a DEMO do Astral. Muito obrigado por jogar!",
        "Espero que tenha gostado do que viu ate agora, novas funcionalidades e conteudos estarao disponiveis na versao completa do jogo.",
        "Nos vemos em breve!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Desenvolvedor", dialogue));

    AddCutscene("endDemo",
                std::move(steps),
                [this](){
                    SetGameScene(GameScene::EndDemo);
                });

    steps.clear();
    dialogue = {
        "Opa, opa, opa...",
        "Esse e o Quasar, um golem de energia que protege o Espaco Astral.",
        "Voce deu sorte dele estar dormindo hoje.",
        "Nao acho que voce vai conseguir acorda-lo, mas se ele acordar, corra!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("quasar_encounter",
                std::move(steps),
                [this](){});

    mAudio->PlaySound("level1Theme.ogg", true);

    mZoe->SetAbilitiesLocked(true);
    StartCutscene("Intro");
}

void Game::LoadTestsLevel()
{
    mHUD = new HUD(this, Game::FONT_PATH_INTER);

    SetApplyGravityScene(true);
    SetCameraCenterToLogicalWindowSizeCenter();

    SetMap("tests.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/galaxy.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    mAudio->PlaySound("level1Theme.ogg", true);
}

void Game::LoadDeathScreen()
{
    UIScreen *mainMenu = new UIScreen(this, FONT_PATH_SMB);

    mainMenu->AddBackground(
        "../assets/Sprites/Menu/backgroundDeath.png",
        Vector2(0, 0),
        Vector2(mWindowWidth, mWindowHeight));

    Vector2 titleSize = Vector2(256.f, 171.f);
    mainMenu->AddImage(
        "../assets/Sprites/Menu/deathText.png",
        Vector2(mWindowWidth / 2.0f - titleSize.x / 2.0f, mWindowHeight * .1f),
        titleSize);

    const Vector2 restartButtonSize = Vector2(
        137.f,
        38.f);

    const Vector2 restartButtonPos = Vector2(
        mWindowWidth / 2.0f - restartButtonSize.x / 2.0f,
        mWindowHeight * .1f + titleSize.y + 50.f);

    mainMenu->AddTransparentButton(
        restartButtonPos,
        restartButtonSize,
        [this]()
        { SetGameScene(GameScene::Level1); });

    mainMenu->AddImage(
        "../assets/Sprites/Menu/restartButton.png",
        restartButtonPos,
        restartButtonSize);

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);

    mAudio->PlaySound("deathTheme.ogg", true);
}

void Game::LoadEndDemoScene()
{
    UIScreen *mainMenu = new UIScreen(this, FONT_PATH_SMB);

    mainMenu->AddBackground(
        "../assets/Sprites/Menu/backgroundEndDemo.png",
        Vector2(0, 0),
        Vector2(mWindowWidth, mWindowHeight));
    
    const Vector2 restartButtonSize = Vector2(
        137.f,
        38.f);

    const Vector2 restartButtonPos = Vector2(
        mWindowWidth / 2.0f - restartButtonSize.x / 2.0f,
        mWindowHeight * .1f);

    mainMenu->AddTransparentButton(
        restartButtonPos,
        restartButtonSize,
        [this]()
        { SetGameScene(GameScene::MainMenu); });

    mainMenu->AddImage(
        "../assets/Sprites/Menu/restartButton.png",
        restartButtonPos,
        restartButtonSize);

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);

    mAudio->PlaySound("endDemoTheme.ogg", true);
}