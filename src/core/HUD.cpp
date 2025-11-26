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
        SDL_Log("Loading life image: %s", path.c_str());

        UIImage* img = AddImage(
            path,
            Vector2(10.0f, 4.0f),
            Vector2(48.0f, 16.0f)
        );
        img->SetEnabled(false);
        mLifeImages.push_back(img);
    }

    mLifeImages[6]->SetEnabled(true);
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