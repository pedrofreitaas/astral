//
// Created by Lucas N. Ferreira on 08/12/23.
//

#pragma once

#include <string>

#include "UIElements/UIScreen.h"

class HUD : public UIScreen
{
public:
    const int POINT_SIZE = 38;
    const int WORD_HEIGHT = 20.0f;
    const int WORD_OFFSET = 25.0f;
    const int CHAR_WIDTH = 20.0f;
    const int HUD_POS_Y = 10.0f;

    HUD(class Game* game, const std::string& fontName);
    ~HUD();

    // Reinsert the HUD elements
    void SetTime(int time);
    void UpdateLives(int lives);
    void UpdateMousePosition(int x, int y);

private:
    // HUD elements
    UIText* mTimeText;

    UIImage* mFrame;
    UIImage* mPunkIcon;
    std::vector<UIImage*> mLives;
    UIText* mProjectileText;

    UIImage* mCursor;

};
