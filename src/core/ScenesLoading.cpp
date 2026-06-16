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
#include "../actors/Mother.h"
#include "../actors/MetalCrate.h"

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
        // [this]() { SetGameScene(GameScene::BedroomFinal); });
    
    mainMenu->AddImage(
        "../assets/Sprites/Menu/playButton.png",
        playButtonPos,
        playButtonSize);

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);

    mMusicHandle = mAudio->PlaySound("mainMenuTheme.ogg", true);
}

void Game::LoadBedroom()
{
    // mHUD = new HUD(this, FONT_PATH_INTER);

    SetMap("bedroom.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);

    SetApplyGravityScene(false);

    new Dog(this, Vector2(300.f, 280.f));

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

    mMusicHandle = mAudio->PlaySound("bedroomTheme.ogg", true);
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
        20.f,
        false,
        .5f));
    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetZoe()->GetCenter() + Vector2(10.f, 0.f); },
        20.f,
        false,
        .5f));
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
        true,
        3.f));
    steps.push_back(std::make_unique<UnspawnStep>(
        this,
        [this]()
        { return this->GetZoe(); }));

    AddCutscene("portalSuck",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::Level1);
                },
                true);
    
    mZoe->SetAbilitiesLocked(true);
    mMusicHandle = mAudio->PlaySound("bedroomTheme.ogg", true);
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

    dialogue = {
        "Oi Zoe! Seja bem vinda ao Espaco Astral!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    dialogue = {
        "Como assim? O que e esse lugar? Quem e voce?",
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(730.f, 330.f); }));

    dialogue = {
        "Espere! Volte aqui!"
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    AddCutscene("Intro",
                std::move(steps),
                [this](){
                    this->GetZoe()->SetAbilitiesLocked(false);
                });

    steps.clear();
    dialogue = {
        "Opa, opa, opa...",
        "Esse parece um Quasar, um golem do Espaco Astral.",
        "Mamae me contava historias sobre ele...",
        "Acho que nao vou conseguir feri-lo.",
        "Talvez tenha outra coisa possa..."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("quasar_encounter",
                std::move(steps),
                [this](){});

    steps.clear();

    dialogue = {
        "Ufa! Essa foi dificil."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(256.f, 160.f)));

    steps.push_back(std::make_unique<WaitStep>(this, 0.5f));

    dialogue = {
        "Muito bem Zoe!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

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
                },
                true);

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
        Vector2(1353.f, 969.f)));

    dialogue = {
        "Acho que ja e hora de eu te contar.",
        "Voce tem ideia de onde esta?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    dialogue = {
        "Sim, eu estou no Espaco Astral.",
        "Minha mae lia historias sobre esse lugar para mim antes de dormir, parece que eu estou dentro do livro agora."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Isso mesmo Zoe, e o seu pai sempre entrava no quarto, bem no final da historia, para dar um susto em voces."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Estrela", dialogue));

    dialogue = {
        "Isso! Ele fingia que era o monstro Zathura.",
        "Mas, espera ai...",
        "Como voce sabe disso?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

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
            auto father = GetFather();
            auto zoe = GetZoe();
            Vector2 zoeCenter = zoe->GetCenter();

            if (Math::Abs(zoeCenter.x - father->GetCenter().x) < 50.f)
                return zoeCenter;

            if (zoeCenter.x > father->GetCenter().x)
                return Vector2(1418.f, zoeCenter.y);

            return zoeCenter;
        },
        80.f,
        false,
        8.f));

    dialogue = {
        "Papai? E voce mesmo? O que voce esta fazendo aqui?",
        "Eu achei que a gente ia para casa da vovo, o que aconteceu?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Eu nao tenho muito tempo para explicar...",
        "Mas as historias que sua mamae lia para voce antes de dormir eram reais.",
        "A gente veio aqui buscar o presente de aniversario da sua Vo.",
        "Mas nao tomamos o cuidado necessario, e Zathura capturou sua mae."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Zathura? O monstro do livro? Ele e real tambem?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Sim Zoe, ele e real. E ele e muito perigoso.",
        "Nos nao temos tempo a perder, voce precisa ficar forte para enfrenta-lo e salvar sua mae.",
        "Para isso, vamos visitar o planeta que ele mora, Nebula.",
        "La voce vai enfrentar problemas maiores e ficar mais forte!",
        "So assim vamos conseguir enfrentar Zathura e salvar sua mae."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Nebula?",
        "A mamae me contava historias sobre os ventos gelidos de la."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("endFirstLevel",
                std::move(steps),
                [this]()
                {
                    SetGameScene(GameScene::Level2);
                },
                true);

    mMusicHandle = mAudio->PlaySound("level1Theme.ogg", true);

    steps.clear();
    dialogue = {
        "Eu acho que posso segurar o meu martelo e bater com muita forca no chao.",
        "Ai eu vou jogar esse Quasar nas Shurikens!"
    };

    steps.push_back(std::make_unique<FreezePhysicsStep>(this));
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));
    steps.push_back(std::make_unique<UnfreezePhysicsStep>(this));

    AddCutscene("quasarEncounterTip",
                std::move(steps));

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

    mMusicHandle = mAudio->PlaySound("level2Theme.ogg", true);

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue;

    new Father(this, Vector2(128.f, 288.f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetFather(); },
        [this](){ 
            auto father = GetFather();
            return father->GetPosition() - Vector2(5.f, 0.f); 
        },
        80.f,
        false,
        1.f));

    dialogue = {
        "Chegamos Zoe!",
        "Esse e Nebula, o planeta onde Zathura mora."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Nossa pai, e igual a mamae contava nas historias!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Sim, mas nos nao temos tempo para ficar admirando a vista."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        [this](){ 
            auto father = GetFather();
            return father->GetCenter(); 
        }));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetFather(); }));

    steps.push_back(std::make_unique<WaitStep>(this, .2f));

    dialogue = {
        "Vamos logo, voce precisa ficar forte para enfrentar Zathura e salvar sua mae."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ 
            return GetPortal()->GetCenter(); },
        280.f));
    
    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    AddCutscene("startSecondLevel",
                std::move(steps),
                [this]()
                {
                    GetZoe()->SetIsFireballAllowed(true);
                    GetZoe()->SetIsVentaniaAllowed(true);
                    Item::CreateNevascaItem(this, Vector2(80.f, 256.f));
                },
                true);

    steps.clear();

    steps.push_back(std::make_unique<SoundStep>(this, "portalSuck.wav"));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetPortal()->GetPosition(); },
        80.f,
        true,
        1.5f));
    
    AddCutscene("portalSuckLevel2",
                std::move(steps),
                [this]()
                {
                    GetZoe()->SetPosition(Vector2(38.f, 620.f));
                },
                true);

    steps.clear();

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(19, 19) * 32));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(19, 20) * 32));

    AddCutscene("breakLevelSithPhaseTiles",
                std::move(steps),
                nullptr,
                true);

    steps.clear();
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(37, 11) * 32));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(38, 11) * 32));

    AddCutscene("breakLevelSithPhaseTiles2",
                std::move(steps),
                nullptr,
                true);

    steps.clear();

    steps.push_back(std::make_unique<SoundStep>(this, "portalSuck.wav"));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetPortal()->GetCenter(); },
        80.f,
        true,
        1.5f,
        true));

    AddCutscene("spawnPortalLevel2",
                std::move(steps),
                [this]()
                {
                    GetZoe()->SetCenter(Vector2(1327.f, 256.f));
                    GetZoe()->SetRotation(0.f);
                },
                true);

    steps.clear();

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Portal,
        Vector2(1847.f, 85.f),
        .5f,
        Math::Pi));

    dialogue = {
        "Esses portais..."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("dontLikePortals",
                std::move(steps));

    steps.clear();
    dialogue = {
        "Talvez eu devesse tentar congelar essa caixa no ar, e usa-la para pegar impulso."
    };
    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    AddCutscene("metalCrateTip",
                std::move(steps));

    steps.clear();

    steps.push_back(std::make_unique<SoundStep>(this, "portalSuck.wav"));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return GetPortal()->GetCenter(); },
        80.f,
        true,
        1.5f,
        true));

    AddCutscene("metalCrateLevelFinal",
                std::move(steps),
                [this]()
                {
                    GetZoe()->SetCenter(Vector2(1330.f, 655.f));
                    GetZoe()->SetRotation(0.f);
                },
                true);

    steps.clear();

    steps.push_back(std::make_unique<SpawnStep>(this, SpawnStep::ActorType::Mother, Vector2(1866.f, 655.f), 0.f));

    steps.push_back(std::make_unique<WaitStep>(this, .75f));

    dialogue = {
        "Mamae?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Oi minha filha, que bom que voce esta bem.",
        "Eu estou tao orgulhosa de voce por ter chegado ate aqui, e por ser tao corajosa.",
        "Onde esta seu pai?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Mae", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(1300.f, 460.f)));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(1432.f, 559.f); },
        250.f));

    dialogue = {
        "Opa, nao se esquecam de mim!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));


    dialogue = {
        "Que bom ver voces de novo!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Mae", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Zathura,
        Vector2(1615.f, 516.f)));

    steps.push_back(std::make_unique<WaitStep>(this, .73f));

    steps.push_back(std::make_unique<FreezePhysicsStep>(this));

    dialogue = {
        "Eu nao quero atrapalhar a reuniao de familia de voces, mas...",
        "Ninguem vai embora daqui sem me enfrentar primeiro!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zathura", dialogue));

    dialogue = {
        "Boa sorte Zoe!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai e Mae", dialogue));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetMother(); }));

    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(49, 17) * 32, .5f));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(51, 17) * 32, .5f));
    steps.push_back(std::make_unique<BreakTileStep>(this, Vector2(50, 17) * 32, .5f));

    steps.push_back(std::make_unique<UnfreezePhysicsStep>(this));

    AddCutscene("motherEncounter",
                std::move(steps),
                nullptr,
                true);

    steps.clear();

    steps.push_back(std::make_unique<WaitStep>(this, 1.25f));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        Vector2(1790.f, 512.f)));
    
    steps.push_back(std::make_unique<WaitStep>(this, .5f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this](){ return GetStar(); },
        [this](){ return Vector2(1600.f, 651.f); },
        250.f));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    steps.push_back(std::make_unique<ShakeStep>(this, .5f, 2));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Father,
        Vector2(1600.f, 651.f)));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Mother,
        Vector2(1630.f, 651.f)));

    steps.push_back(std::make_unique<WaitStep>(this, .5f));

    dialogue = {
        "Que bom que voce conseguiu derrotar Zathura, Zoe!",
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai e Mae", dialogue));

    dialogue = {
        "Obrigada pai, obrigada mae. Eu nao teria conseguido sem voces.",
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Vamos voltar para casa, voces merecem um descanso depois de tudo isso."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Portal,
        Vector2(1660.f, 512.f)));

    steps.push_back(std::make_unique<WaitStep>(this, .5f));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetMother(); },
        [this]()
        { return Vector2(1660.f, 512.f); },
        220.f,
        true,
        3.f,
        true));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetMother(); }));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetFather(); },
        [this]()
        { return Vector2(1660.f, 512.f); },
        220.f,
        true,
        3.f,
        true));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetFather(); }));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return Vector2(1660.f, 512.f); },
        220.f,
        true,
        3.f,
        true));

    steps.push_back(std::make_unique<FreezePhysicsStep>(this));

    AddCutscene("postDefeatZathura",
                std::move(steps),
                [this]()
                {
                    SetGameScene(GameScene::BedroomFinal);
                },
                true);

    steps.clear();

    steps.push_back(std::make_unique<ApplyKnockbackStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        {
            auto zoe = GetZoe();
            float zoeX = zoe->GetCenter().x;
            
            auto zathura = GetZathura();
            float zathuraX = zathura->GetCenter().x;

            if (zoeX >= 1440 && zoeX <= 1760)
            {
                int diffXSign = Math::Sign(zoeX - zathuraX);
                return Vector2(diffXSign * 500.f, -150.f);
            }
            
            return Vector2(0,0);
        }));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    dialogue = {
        "Voce nao vai se safar dessa!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zathura", dialogue));

    Vector2 spawn = Vector2(1600.f, 480.f);

    steps.push_back(std::make_unique<SpawnStep>(
        this,
        SpawnStep::ActorType::Star,
        spawn));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));
        
    dialogue = {
        "Calma Zoe, eu te ajudo com essa!"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

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
        [this, starHexagon, spawn]()
        {
            return spawn + starHexagon[0];
        },
        120.f));

    int totalSpins = 5;
    for (int i=1; i<totalSpins*starHexagon.size(); i+=2) {
        steps.push_back(std::make_unique<MoveStep>(
            this,
            [this](){ return GetStar(); },
            [this, i, starHexagon, spawn]()
            {
                Vector2 offset = starHexagon[i % starHexagon.size()];

                return spawn + offset;
            },
            30.f * (i+1)));
    }

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetStar(); },
        [this]()
        { return GetZathura()->GetCenter(); },
        totalSpins * starHexagon.size() * 30.f));

    steps.push_back(std::make_unique<ShakeStep>(this, .5f, 5));

    steps.push_back(std::make_unique<UnspawnStep>(
        this, [this]()
        { return GetStar(); }));

    AddCutscene("defeatZathura",
                std::move(steps),
                [this]()
                {
                    GetZathura()->DeathCutscenePlayedCallback();
                    StartCutscene("postDefeatZathura");
                },
                true);

    StartCutscene("startSecondLevel");
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

    mMusicHandle = mAudio->PlaySound("level1Theme.ogg", true);
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
        { SetGameScene(mPreviousScene); });

    mainMenu->AddImage(
        "../assets/Sprites/Menu/restartButton.png",
        restartButtonPos,
        restartButtonSize);

    mUIStack.front()->AddCursor(
        "../assets/Sprites/Hud/cursor.png",
        Vector2(0.0f, 0.0f),
        Vector2(33.0f, 33.0f),
        Color::White);

    mMusicHandle = mAudio->PlaySound("deathTheme.ogg", true);
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

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue;

    dialogue = {
        "E foi assim que a Zoe conseguiu salvar a mae dela e derrotar Zathura.",
        "Ela e muito corajosa, nao e mesmo?",
        "Parabens por chegar ate aqui! Voce terminou a versao final do jogo.",
        "Muito obrigado por jogar, espero que voce tenha gostado!",
        "Se voce quiser saber mais sobre a criacao jogo, basta continuar lendo.",
        "Se nao, voce pode clicar no botao para voltar para o menu principal e jogar de novo, ou ate mesmo mostrar o jogo para um amigo!",
        "Astral foi um jogo que nasceu do amor que eu tenho por jogos de plataforma 2D, e da minha vontade de criar um jogo proprio.",
        "Ele e resultado de um trabalho de conclusao de curso (TCC), que durou aproximadamente 1 ano ao todo.",
        "Eu sou Pedro Oliveira, e desenvolvedor do Astral. Eu sou apaixonado por jogos desde crianca, e sempre sonhei em criar um jogo proprio.",
        "Nesse processo, fui orientado pelo Lucas Ferreira, professor da UFMG.",
        "Obrigado a todos que me ajudaram a testar esse jogo, nos varios playtest que realizei.",
        "Um agradecimento especial para o Arthur Lana, mais creditos no GDD, que ajudou com o QA profundo do jogo.",
        "Varios dos assets do jogo nao foram criados por mim, todos os creditos sao dados no GDD do Astral.",
        "Voce pode encontra-lo no Itch.io do jogo: 'https://pedrofreitaas.itch.io/astral'",
        "Alguns easter eggs:",
        "Os nomes desse jogo sao inspirados em personagens de obras espacias: Zod (o sentinela atirador),",
        "Zathura (o monstro do livro), Nebula (o planeta do segundo nivel),",
        "Sith (a besta voadora) e o Quasar (o golem do segundo nivel)",
        "Originalmente, o roteiro do jogo era um pouco diferente, a estrela nao era o Pai de Zoe, e o final do jogo nem parecia com esse final,",
        "Mas eu acabei mudando algumas coisas no processo de desenvolvimento para deixar a historia mais fluida, interessante.",
        "Fazer um jogo e um processo de muita tentativa e erro, e eu acabei mudando varias coisas no roteiro original, mas eu estou muito feliz com o resultado final.",
        "E um grande desafio, mas sem duvidas, muito divertido e gratificante.",
        "Mas, tudo tem seu fim."
        "E com MUITO PESAR, eu encerro Astral por aqui. Muito obrigado por jogar!",
        "Enfim, ate a proxima aventura! E cuidado com Zathura..."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Desenvolvedor", dialogue));

    AddCutscene("credits",
                std::move(steps));

    StartCutscene("credits");

    mMusicHandle = mAudio->PlaySound("endDemoTheme.ogg", true);
}

void Game::LoadBedroomFinal()
{
    SetMap("bedroomFinal.json");

    SetCameraCenter(mMap->GetCenter());
    SetMaintainCameraInMap(false);
    SetApplyGravityScene(false);

    new Dog(this, Vector2(402.f, 75.f));

    mZoe->SetAbilitiesLocked(true);

    std::vector<std::unique_ptr<Step>> steps;
    std::vector<std::string> dialogue;

    dialogue = {
        "Ufa, ainda bem que estamos bem.",
        "Eu estava com medo de que o Zathura fosse forte demais e que eu nao conseguisse derrotar ele."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Zathura? E aquele monstro do livro da sua mae?",
        "Agora eu entendi por que voce demorou tanto para acordar."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    dialogue = {
        "Como assim? Voce nao lembra que acabamos de salvar a mamae, Pai?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    dialogue = {
        "Ah, Zoe... Acho que esse seu tempo extra na cama te rendeu um bom sonho...",
        "Vamos, va pegar o presente da vovo que deixamos no quarto."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Mae", dialogue));

    dialogue = {
        "Ah, ta bom entao.",
        "Pelo menos tudo nao passou de um sonho."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Zoe", dialogue));

    steps.push_back(std::make_unique<MoveStep>(
        this,
        [this]()
        { return GetZoe(); },
        [this]()
        { return Vector2(474.f, 103.f); },
        80.f,
        false,
        1.2f));

    steps.push_back(std::make_unique<UnspawnStep>(
        this,
        [this]()
        { return GetZoe(); }));

    dialogue = {
        "Sera que e mesmo a melhor decisao esconder isso dela?"
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Mae", dialogue));

    dialogue = {
        "Nao sei.",
        "Mas tenho certeza que nao devemos mais voltar para la."
    };

    steps.push_back(std::make_unique<DialogueStep>(this, "Pai", dialogue));

    steps.push_back(std::make_unique<WaitStep>(this, 1.f));

    AddCutscene("finalCutscene",
                std::move(steps),
                [this]()
                {
                    this->SetGameScene(GameScene::EndDemo);
                });

    mMusicHandle = mAudio->PlaySound("bedroomTheme.ogg", true);
}