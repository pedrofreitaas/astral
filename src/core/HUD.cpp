#include "HUD.h"
#include "Game.h"
#include "../ui/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
{
    // FPS
    mFPSText = AddText(
        "60", 
        Vector2(mGame->GetWindowWidth() - 425.0f + CHAR_WIDTH, 35.0f), 
        Vector2(CHAR_WIDTH * 3, WORD_HEIGHT), 
        POINT_SIZE, 1024, 
        Color::Red
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
