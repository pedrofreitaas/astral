//
// Created by Lucas N. Ferreira on 08/12/23.
//

#pragma once

#include <string>

#include "../ui/UIScreen.h"

class HUD : public UIScreen
{
public:
    const int POINT_SIZE = 38;

    HUD(class Game* game, const std::string& fontName);
    ~HUD();

    void SetFPS(int fps);
    void SetFPS(float fps);

    void HandleKeyPress(int key) override;
	void HandleMouseClick(int button, int x, int y) override;

private:
    UIText* mFPSText;
};
