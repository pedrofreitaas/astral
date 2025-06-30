//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "Game.h"
#include "UIElements/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    :UIScreen(game, fontName)
{
    // Initialize HUD elements
        AddText("Time", Vector2(mGame->GetWindowWidth() - 150.0f, 20.0f + HUD_POS_Y), Vector2(CHAR_WIDTH * 4, WORD_HEIGHT), POINT_SIZE);
        mTimeText = AddText("100", Vector2(mGame->GetWindowWidth() - 150.0f + CHAR_WIDTH, 20.0f + HUD_POS_Y + WORD_OFFSET), Vector2(CHAR_WIDTH * 3, WORD_HEIGHT), POINT_SIZE);

        AddText("Lives", Vector2(40.0f, 20.0f + HUD_POS_Y), Vector2(CHAR_WIDTH * 5, WORD_HEIGHT), POINT_SIZE);
        mLives = AddText("5", Vector2(80.0f, 20.0f + HUD_POS_Y + WORD_OFFSET), Vector2(CHAR_WIDTH, WORD_HEIGHT), POINT_SIZE);
}

HUD::~HUD()
{

}

void HUD::SetTime(int time)
{
    mTimeText->SetText(std::to_string(time));

    // Reinsert position and size based on the number of digits
    int numDigits = std::to_string(time).length();
    mTimeText->SetPosition(Vector2(mGame->GetWindowWidth() - 150.0f + (4 - numDigits) * CHAR_WIDTH, 20.0f + HUD_POS_Y + WORD_OFFSET));
    mTimeText->SetSize(Vector2(CHAR_WIDTH * numDigits, WORD_HEIGHT));
}
void HUD::UpdateLives(int lives) {
    mLives->SetText(std::to_string(lives));
}

void HUD::SetLevelName()
{

}