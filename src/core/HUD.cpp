#include "HUD.h"
#include "Game.h"
#include "../ui/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
{
    int width = 64, height = 64;
    mFPSText = AddText(
        "fps", 
        Vector2(mGame->GetWindowWidth()-width, 0.0f), 
        Vector2(width, height), 
        POINT_SIZE, 
        1024, 
        Color::Blue
    );
    mFPSText->SetEnabled(false);
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