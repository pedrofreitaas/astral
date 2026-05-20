#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>
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
#include "../actors/Dog.h"
#include "../actors/Father.h"

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
        [this]() { SetGameScene(GameScene::Bedroom); });
        // [this]() { SetGameScene(GameScene::Tests); });
    
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

    auto *dog = new Dog(this, Vector2(300.f, 280.f));

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue = {
        "Hoje e dia de visitar a Vovo, Papai e Mamae ja devem estar se arrumando."};
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
                }, 
                true);

    mZoe->SetAbilitiesLocked(true);

    mAudio->PlaySound("bedroomTheme.ogg", true);
}

void Game::LoadBedroomPortal()
{
    SetMap("bedroomPortal.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    auto portal = new Portal(this, Vector2(0.f, 72.f) + mMap->GetCenter());

    std::vector<std::unique_ptr<Step>> steps;

    steps.push_back(std::make_unique<WaitStep>(this, .1f));
    AddCutscene("leaveBedroomPortal",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::Bedroom);
                },
                true);

    steps.clear();

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

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetPortal(); }));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x + 64.f, GetZoe()->GetCenter().y);},
        80.f));

    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x - 10.f, GetZoe()->GetCenter().y);},
        20.f));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){return Vector2(GetZoe()->GetCenter().x + 3.f, GetZoe()->GetCenter().y);},
        20.f));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    dialogue = {
        "O que... o que e isso?",
        "Onde estou? Parece que estou no espaco."};
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
        "Uma estrela? Ela parece estar indo para algum lugar?"};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(730.f, 330.f); }));

    dialogue = {
        "Espere! Volte aqui!"};
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    dialogue = {
        "Oi Zoe! Seja bem vinda ao Espaco Astral!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    dialogue = {
        "Como assim? O que e esse lugar? Quem e voce?",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Se acalme garota... Cuidado para nao cair!",
        "Acho que voce sabe o que fazer, boa sorte!"
    };
    
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("Intro",
                std::move(steps),
                [this](){
                    this->GetZoe()->SetAbilitiesLocked(false);
                });

    steps.clear();
    dialogue = {
        "Opa, opa, opa...",
        "Esse e o Quasar, uma especie de golem que vaga o Espaco Astral.",
        "Nao acho que voce vai conseguir feri-lo.",
        "Talvez tenha outra coisa possa..."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    AddCutscene("quasar_encounter",
                std::move(steps),
                [this](){});

    steps.clear();

    dialogue = {
        "Ufa! Essa foi dificil."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Muito bem Zoe!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(256.f, 160.f)));

    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));

    dialogue = {
        "Opa, olha so quem apareceu de novo!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Narrador", dialogue));

    std::vector<Vector2> starHexagon = {
        Vector2(-30, 0),
        Vector2(-21.3, 21.3),
        Vector2(0, 30),
        Vector2(21.3, 21.3),
        Vector2(30, 0),
        Vector2(21.3, -21.3),
        Vector2(0, -30),
        Vector2(-21.3, -21.3)
    };

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this, starHexagon]()
        {
            return GetZoe()->GetCenter() + starHexagon[0];
        },
        120.f));

    int totalSpins = 5;
    for (int i=1; i<totalSpins*starHexagon.size(); i++) {
        steps.push_back(std::make_unique<MoveStep>(
            this,
            [this](){ return GetStar(); },
            [this, i, starHexagon]()
            {
                Vector2 offset = starHexagon[i % starHexagon.size()];

                return GetZoe()->GetCenter() + offset;
            },
            30.f * (i+1)));
    }

    steps.push_back(std::make_unique<ShakeStep>(this, .5f, 5));

    AddCutscene("halfFirstLevel",
                std::move(steps),
                [this]()
                {
                    GetZoe()->TeleportToSecondHalfLevel1();
                });

    steps.clear();

    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(39, 23) * 32));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(39, 24) * 32));
    
    dialogue = {
        "Parece que la em cima abriu!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    AddCutscene("breakTiles",
                std::move(steps));

    steps.clear();

    steps.push_back(std::make_unique<WaitStep>(this, 1.5f));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(1290.f, 768.f)));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ 
            auto zoe = GetZoe();
            Vector2 zoeCenter = zoe->GetCenter();

            if (zoeCenter.x < 1580.f) {
                return zoeCenter + Vector2(50.f, -10.f);
            }
            
            return zoeCenter + Vector2(-50.f, -10.f);
        },
        140.f,
        false,
        8.f));

    dialogue = {
        "Voce ja se mostrou mais do que preparada.",
        "Acho que ja e hora de eu te contar.",
        "Voce tem ideia de onde esta?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    dialogue = {
        "Sim, eu estou no Espaco Astral.",
        "Igual ao que minha mamae lia historias para mim antes de dormir, parece que eu estou dentro do livro agora."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Isso mesmo Zoe, e o seu pai sempre entrava no quarto, bem no final da historia, para dar um susto em voces."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    dialogue = {
        "Isso! Ele fingia que era o monstro Zathura.",
        "Mas, pera...",
        "Como voce sabe disso?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Ja esta na hora de voce saber!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(1353.f, 969.f); },
        140.f,
        false,
        8.f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){
            auto zoe = GetZoe();
            auto star = GetStar();

            Vector2 zoeCenter = zoe->GetCenter();
            Vector2 starCenter = star->GetCenter();

            if (Math::Abs(zoeCenter.x - starCenter.x) < 30.f) {
                return zoeCenter + Vector2(150.f, 0.f);
            }
            
            return zoeCenter;
        },
        60.f));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Father,
        Vector2(1353.f, 969.f)));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    dialogue = {
        "Oi Zoe!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetZoe(); },
        [this](){ 
            auto zoe = GetZoe();
            Vector2 zoeCenter = zoe->GetCenter();
            return Vector2(1418.f, zoeCenter.y); 
        },
        60.f));

    dialogue = {
        "Pai? E voce mesmo? O que voce esta fazendo aqui?",
        "Eu achei que a gente ia para casa da vovo, o que aconteceu?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Eu nao tenho muito tempo para explicar...",
        "Mas as historias que sua mamae lia para voce antes de dormir, eram reais.",
        "A sua mae adora esse lugar, mas ele pode ser perigoso!",
        "A gente veio aqui buscar o presente de aniversario da sua Vo. Mas a gente nao tomou o cuidado necessario.",
        "E Zathura capturou sua mae."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Zathura? O monstro do livro? Ele e real tambem?",
        "Nossa, isso e tao assustador... E o que a gente faz agora? Eu preciso salvar minha mae!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Sim Zoe, ele e real. E ele e muito perigoso.",
        "Nos nao temos tempo a perder, voce precisa ficar forte para enfrentar ele e salvar sua mae.",
        "Para isso, vamos visitar o planeta que ele mora, que se chama Nebula.",
        "La voce vai enfrentar problemas maiores e ficar mais forte!",
        "So assim vamos conseguir enfrentar Zathura e salvar sua mae."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Nebula?",
        "A historia que mamae me contava falava sobre os ventos gelidos de la.",
        "Se e para la que eu tenho que ir, entao vamos logo! Eu quero salvar minha mae!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("endFirstLevel",
                std::move(steps),
                [this]()
                {
                    SetGameScene(GameScene::Level2);
                });

    mAudio->PlaySound("level1Theme.ogg", true);

    mZoe->SetAbilitiesLocked(true);
    StartCutscene("Intro");
                
    Item::CreateVentaniaItem(this, Vector2(836,608));
    Item::CreateFireballItem(this, Vector2(1377,800));
}

void Game::LoadSecondLevel()
{
    mHUD = new HUD(this, Game::FONT_PATH_INTER);

    SetApplyGravityScene(true);
    SetCameraCenterToLogicalWindowSizeCenter();

    SetMap("level2.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/nebula.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    mAudio->PlaySound("level2Theme.ogg", true);

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue;

    dialogue = {
        "Seja bem vindo(a), essa e uma fase conceitual do segundo nivel.",
        "Ele ainda esta em desenvolvimento, nao tem nada por aqui. Muito obrigado por jogar o primeiro nivel!",
        "Espero te ver novamente quando o segundo estiver pronto :)"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Desenvolvedor", dialogue));
    AddCutscene("explainLevel2Wip",
                std::move(steps));
}

void Game::LoadTestsLevel()
{
    mHUD = new HUD(this, Game::FONT_PATH_INTER);

    SetApplyGravityScene(true);
    SetCameraCenterToLogicalWindowSizeCenter();

    SetMap("tests.json");

    SetBackgroundImage(
        "../assets/Levels/Backgrounds/nebula.png",
        Vector2(0.0f, 0.0f),
        Vector2(mWindowWidth, mWindowHeight),
        false);

    mAudio->PlaySound("level1Theme.ogg", true);

    new Father(this, Vector2(295.f, 160.f));
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