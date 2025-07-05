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

    // Timer
    AddImage("../Assets/Sprites/Hud/frame.png", Vector2(mGame->GetWindowWidth() - 145.0f, 10.0f), Vector2(140.0f, 56.0f));
    // AddText("Time", Vector2(mGame->GetWindowWidth() - 150.0f, 20.0f + HUD_POS_Y), Vector2(CHAR_WIDTH * 4, WORD_HEIGHT), POINT_SIZE);
    mTimeText = AddText("100", Vector2(mGame->GetWindowWidth() - 125.0f + CHAR_WIDTH, 35.0f), Vector2(CHAR_WIDTH * 3, WORD_HEIGHT), POINT_SIZE, 1024, Color::White);

    // Hud
    // Frame de fundo
    mFrame = AddImage("../Assets/Sprites/Hud/hud.png", Vector2(10.0f, 10.0f), Vector2(169.0f, 70.0f));
    // Fotinha do Punk
    mPunkIcon = AddImage("../Assets/Sprites/Hud/punk.png", Vector2(18.0f, 32.0f), Vector2(38.0f, 38.0f));
    // Vidas (6)
    for (int i = 0; i < 6; ++i) {
        AddImage("../Assets/Sprites/Hud/life_empty.png", Vector2(70.0f + i * 17.0f, 20.0f), Vector2(16.5f, 8.0f));
    }
    for (int i = 0; i < 6; ++i) {
        mLives.push_back(AddImage("../Assets/Sprites/Hud/life_full.png", Vector2(70.0f + i * 17.0f, 20.0f), Vector2(16.5f, 8.0f)));
    }

    // Projectile - TODO
    AddImage("../Assets/Sprites/Hud/gun.png", Vector2(75.0f, 50.0f), Vector2(28.0f, 20.0f));
    //mProjectileText = AddText("10", Vector2(65.0f + 32.0f, 32.0f), Vector2(CHAR_WIDTH * 2, WORD_HEIGHT), POINT_SIZE-10, 1024, Color::White);


    // Cursor
    mCursor = AddCursor("../Assets/Sprites/Hud/cursor.png", Vector2(0, 0), Vector2(15, 21));
    
}

HUD::~HUD()
{
}

void HUD::SetTime(int time)
{
    mTimeText->SetText(std::to_string(time));

    // // Reinsert position and size based on the number of digits
    int numDigits = std::to_string(time).length();
    mTimeText->SetPosition(Vector2(mGame->GetWindowWidth() - 135.0f + (4 - numDigits) * CHAR_WIDTH, 35.0f));
    mTimeText->SetSize(Vector2(CHAR_WIDTH * numDigits, WORD_HEIGHT));
}
void HUD::UpdateLives(int lives) {
    while(lives < mLives.size()) {
        mLives[lives]->~UIImage();
        mLives.pop_back();
    }
}

void HUD::UpdateMousePosition(int x, int y) {
    mCursor->SetPosition(Vector2(static_cast<float>(x), static_cast<float>(y)));
}
