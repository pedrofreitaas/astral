#include "HUD.h"
#include "Game.h"
#include "../ui/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
{
    int width = 64, height = 64;
    // FPS
    mFPSText = AddText(
        "fps", 
        Vector2(mGame->GetWindowWidth()-width, 0.0f), 
        Vector2(width, height), 
        POINT_SIZE, 
        1024, 
        Color::Blue
    );
}

HUD::~HUD()
{
}

void HUD::SetFPS(int fps)
{
    int displayFPS = std::max(0, std::min(999, fps));
    mFPSText->SetText(std::to_string(displayFPS));
}
