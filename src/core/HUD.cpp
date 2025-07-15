//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "Game.h"
#include "../ui/UIText.h"

HUD::HUD(class Game* game, const std::string& fontName)
    : UIScreen(game, fontName), mAmmo(), mAmmoBackground(), mGunName("Unkown"), mGunIcon(nullptr)
{
    // Initialize HUD elements

    // Timer
    AddImage("../assets/Sprites/Hud/frame.png", Vector2(mGame->GetWindowWidth() - 145.0f, 10.0f), Vector2(140.0f, 56.0f));
    mTimeText = AddText("400", Vector2(mGame->GetWindowWidth() - 125.0f + CHAR_WIDTH, 35.0f), Vector2(CHAR_WIDTH * 3, WORD_HEIGHT), POINT_SIZE, 1024, Color::White);

    // Hud
    // Frame de fundo
    mFrame = AddImage("../assets/Sprites/Hud/hud.png", Vector2(10.0f, 10.0f), Vector2(169.0f, 70.0f));
    // Fotinha do Punk
    mPunkIcon = AddImage("../assets/Sprites/Hud/punk.png", Vector2(18.0f, 32.0f), Vector2(38.0f, 38.0f));
    // Vidas (6)
    for (int i = 0; i < 6; ++i) {
        AddImage("../assets/Sprites/Hud/life_empty.png", Vector2(70.0f + i * 17.0f, 20.0f), Vector2(16.5f, 8.0f));
    }
    for (int i = 0; i < 6; ++i) {
        mLives.push_back(AddImage("../assets/Sprites/Hud/life_full.png", Vector2(70.0f + i * 17.0f, 20.0f), Vector2(16.5f, 8.0f)));
    }

    // Cursor
    mCursor = AddCursor("../assets/Sprites/Hud/cursor.png", Vector2(0, 0), Vector2(15, 21));
}

HUD::~HUD()
{
}
// Em HUD.cpp

void HUD::SetTime(int time)
{
    int displayTime = std::max(0, time);

    mTimeText->SetText(std::to_string(displayTime));

    int numDigits = std::to_string(displayTime).length();

    if (displayTime == 0) {
        numDigits = 1;
    }

    mTimeText->SetPosition(Vector2(mGame->GetWindowWidth() - 130.0f + (4 - numDigits) * CHAR_WIDTH, 35.0f));
    mTimeText->SetSize(Vector2(CHAR_WIDTH * numDigits, WORD_HEIGHT));
}

void HUD::UpdateLives(int lives) {
    const int maxLives = 6;
    if (lives > maxLives) {
        lives = maxLives;
    }

    while (lives > mLives.size()) {
        float newHeartX = 70.0f + static_cast<float>(mLives.size()) * 17.0f;
        Vector2 newHeartPos = Vector2(newHeartX, 20.0f);
        UIImage* newLife = AddImage("../assets/Sprites/Hud/life_full.png", newHeartPos, Vector2(16.5f, 8.0f));
        mLives.push_back(newLife);
    }
    while(lives < mLives.size()) {
        mLives[lives]->~UIImage();
        mLives.pop_back();
    }
}

void HUD::UpdateAmmo(int ammo, int maxAmmo) {
    if (ammo == mAmmo.size() && maxAmmo == mAmmoBackground.size()) {
        return;
    }
    
    while (ammo < mAmmo.size()) {
        mAmmo[mAmmo.size()-1]->~UIImage();
        mAmmo.pop_back();
    }

    while (maxAmmo < mAmmoBackground.size()) {
        mAmmoBackground[mAmmoBackground.size()-1]->~UIImage();
        mAmmoBackground.pop_back();
    }

    const float ammoWidth = 22.0f;

    for (int i=mAmmoBackground.size(); i < maxAmmo; ++i) {
        Vector2 ammoPos = Vector2(70.0f + ammoWidth * i, 80.0f);        
        mAmmoBackground.push_back( AddImage("../assets/Sprites/Hud/ammo_backg.png", ammoPos, Vector2(16.0f, 5.0f)) );
    }

    for (int i=mAmmo.size(); i < ammo; ++i) {
        Vector2 ammoPos = Vector2(70.0f + ammoWidth * i, 80.0f);
        mAmmo.push_back( AddImage("../assets/Sprites/Hud/ammo.png", ammoPos, Vector2(16.0f, 5.0f)) );
    }
}

void HUD::UpdateGun(std::string gunName) {
    if (mGunName == gunName) return;
    if (gunName == "Unkown") return;

    mGunName = gunName;
    if (mGunIcon) mGunIcon->~UIImage();

    if (gunName == "Pistol") {
        mGunIcon = AddImage("../assets/Sprites/Hud/pistol.png", Vector2(80.0f, 55.0f), Vector2(11.0f, 9.0f));
        return;
    } 
    
    if (gunName == "Shotgun") {
        mGunIcon = AddImage("../assets/Sprites/Hud/shotgun.png", Vector2(70.0f, 55.0f), Vector2(26.0f, 7.0f));
        return;
    }

    SDL_Log("Didn't find gun for HUD update.");
}

void HUD::UpdateMousePosition(int x, int y) {
    mCursor->SetPosition(Vector2(static_cast<float>(x), static_cast<float>(y)));
}
