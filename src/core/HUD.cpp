#include "HUD.h"
#include "Game.h"
#include "../ui/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
{
    int width = 64, height = 64;

    mFPSText = AddText(
        "fps",
        Vector2(mGame->GetWindowWidth() - width, 0.0f),
        Vector2(width, height),
        POINT_SIZE,
        1024,
        Color::Blue
    );
    mFPSText->SetEnabled(false);

    mLifeImages.reserve(7);

    // index 0..6  -> life value 0..6
    std::vector<std::string> lifeImagePaths = {
        
        "../assets/Sprites/Hud/lifeBar/life-bar0.png", // 0
        "../assets/Sprites/Hud/lifeBar/life-bar1.png", // 1
        "../assets/Sprites/Hud/lifeBar/life-bar2.png", // 2
        "../assets/Sprites/Hud/lifeBar/life-bar3.png", // 3
        "../assets/Sprites/Hud/lifeBar/life-bar4.png", // 4
        "../assets/Sprites/Hud/lifeBar/life-bar5.png", // 5
        "../assets/Sprites/Hud/lifeBar/life-bar6.png", // 6
    };

    for (const auto& path : lifeImagePaths)
    {
        UIImage* img = AddImage(
            path,
            Vector2(10.0f, 4.0f),
            Vector2(48.0f, 16.0f)
        );
        img->SetEnabled(false);
        mLifeImages.push_back(img);
    }

    mLifeImages[6]->SetEnabled(true);

    std::vector<std::string> loadingBarPaths = {
        "../assets/Sprites/Hud/loadingBar/loading-bar7.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar6.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar5.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar4.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar3.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar2.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar1.png",
        "../assets/Sprites/Hud/loadingBar/loading-bar0.png"
    };

    for (const auto& path : loadingBarPaths)
    {
        UIImage* img = AddImage(
            path,
            Vector2(.0f, .0f),
            Vector2(16.0f, 16.0f)
        );
        img->SetEnabled(false);
        mLoadingBarImages.push_back(img);
    }

    mFireballLoadingBarOffset = Vector2(-24.0f, -24.0f);
}

HUD::~HUD()
{
}

void HUD::HandleKeyPress(int key)
{
    // Handle key press events
    if (key == SDLK_f)
    {
        mFPSText->SetEnabled(!mFPSText->IsEnabled());
    }
}

void HUD::HandleMouseClick(int button, int x, int y)
{

}

void HUD::SetFPS(int fps)
{
    int displayFPS = std::max(0, std::min(999, fps));
    mFPSText->SetText(std::to_string(displayFPS));
}

void HUD::SetFPS(float fps)
{
    float displayFPS = std::max(0.f, std::min(999.f, fps));
    mFPSText->SetText(std::to_string(displayFPS));
}

void HUD::SetLife(int life) {
    life = std::max(0, std::min(life, 6));
    
    for (int i = 0; i < mLifeImages.size(); ++i) {
        mLifeImages[i]->SetEnabled(i == life);
    }
}

void HUD::SetFireballLoadingBarProgress(bool enabled, float progress) {
    progress = std::max(0.f, std::min(1.f, progress));
    
    int index = static_cast<int>(progress * (mLoadingBarImages.size() - 1));
    index = std::max(0, std::min(index, static_cast<int>(mLoadingBarImages.size() - 1)));

    for (int i = 0; i < mLoadingBarImages.size(); ++i) {
        mLoadingBarImages[i]->SetEnabled(enabled && (i == index));
    }
}

void HUD::UpdateFireballLoadingBar(bool enabled, float progress, Vector2 pos) {
    SetFireballLoadingBarProgress(enabled, progress);
    
    for (int i = 0; i < mLoadingBarImages.size(); ++i) {
        mLoadingBarImages[i]->SetPosition(pos);
    }
}

void HUD::Update(float deltaTime) {
    Zoe* zoe = dynamic_cast<Zoe*>(mGame->GetZoe());

    if (!zoe) {
        SDL_Log("Zoe actor not found in HUD update.");
        return;
    }
    
    SetFPS(static_cast<int>(1.0f / deltaTime));

    SetLife(zoe->GetLifes());

    UpdateFireballLoadingBar(
        zoe->CheckFireballOnCooldown(),
        zoe->GetFireballCooldownProgress(),
        zoe->GetCenter() + mFireballLoadingBarOffset);
}