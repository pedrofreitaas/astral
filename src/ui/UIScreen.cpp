// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "UIScreen.h"
#include "../core/Game.h"
#include "UIFont.h"

UIScreen::UIScreen(Game* game, const std::string& fontName)
	:mGame(game)
	,mPos(0.f, 0.f)
	,mSize(0.f, 0.f)
	,mState(UIState::Active)
    ,mSelectedButtonIndex(-1)
{
	// Add to UI Stack
	mGame->PushUI(this);

    // Load default font
    mFont = mGame->LoadFont(fontName);
}

UIScreen::~UIScreen()
{
    for (auto t : mTexts) {
        delete t;
    }
    mTexts.clear();

    for (auto b : mButtons) {
        delete b;
    }
	mButtons.clear();

    for (auto img : mImages) {
        delete img;
    }
    mImages.clear();

    for (auto img : mCursors) {
        delete img;
    }
    mCursors.clear();

    for (auto img : mBackground) {
        delete img;
    }
    mBackground.clear();
}

void UIScreen::Update(float deltaTime)
{
	
}

void UIScreen::Draw(SDL_Renderer *renderer)
{
    for (auto t : mBackground) {
        t->Draw(renderer, mPos);
    }

    for (auto img : mImages) {
        img->Draw(renderer, mPos);
    }

    for (auto b : mButtons) {
        b->Draw(renderer, mPos);
    }
    
    for (auto t : mTexts) {
        t->Draw(renderer, mPos);
    }

    for (auto t : mCursors) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        Vector2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
        
        t->Draw(renderer, mousePos);
    }
}

void UIScreen::ProcessInput(const uint8_t* keys)
{

}

void UIScreen::HandleKeyPress(int key)
{
    if(key == SDLK_w)
    {
        if (mSelectedButtonIndex >= 0) {
            mButtons[mSelectedButtonIndex]->SetHighlighted(false);

            mSelectedButtonIndex--;
            if(mSelectedButtonIndex < 0) {
                mSelectedButtonIndex = mButtons.size() - 1;
            }

            mButtons[mSelectedButtonIndex]->SetHighlighted(true);
        }
    }
    else if(key == SDLK_s)
    {
        if (mSelectedButtonIndex >= 0) {
            mButtons[mSelectedButtonIndex]->SetHighlighted(false);

            mSelectedButtonIndex++;
            if(mSelectedButtonIndex >= mButtons.size()) {
                mSelectedButtonIndex = 0;
            }

            mButtons[mSelectedButtonIndex]->SetHighlighted(true);
        }
    }
    else if(key == SDLK_RETURN)
    {
        if (mSelectedButtonIndex >= 0 && mSelectedButtonIndex < mButtons.size()) {
            mButtons[mSelectedButtonIndex]->OnClick();
        }
    }
}

void UIScreen::Close()
{
	mState = UIState::Closing;
}

UIText* UIScreen::AddText(const std::string &name, const Vector2 &pos, const Vector2 &dims, const int pointSize, const int unsigned wrapLength, const Vector3 &color)
{
    auto t = new UIText(name, mFont, pointSize, wrapLength, pos, dims, color);
    mTexts.emplace_back(t);
    return t;
}

UIButton* UIScreen::AddButton(const std::string& name, const Vector2 &pos, const Vector2& dims, std::function<void()> onClick, const Vector2& textSize)
{
    auto b = new UIButton(
        name, mFont, onClick,
        pos, dims, 
        Vector3(30, 30, 30), 40, 1024, 
        Vector2::Zero, textSize, Color::White
    );
	mButtons.emplace_back(b);

    // Set the first button as highlighted
    if(mButtons.size() == 1) {
        mSelectedButtonIndex = 0;
        b->SetHighlighted(true);
    }

    return b;
}

UIImage* UIScreen::AddImage(const std::string &imagePath, const Vector2 &pos, const Vector2 &dims, const Vector3 &color)
{
    auto img = new UIImage(imagePath, pos, dims, color);
    mImages.emplace_back(img);
    return img;
}

UIImage* UIScreen::AddCursor(const std::string &imagePath, const Vector2 &pos, const Vector2 &dims, const Vector3 &color)
{
    auto img = new UICursor(imagePath, pos, dims, color);
    mCursors.emplace_back(img);
    return img;
}

UIImage* UIScreen::AddBackground(const std::string &imagePath, const Vector2 &pos, const Vector2 &dims, const Vector3 &color)
{
    auto img = new UIImage(imagePath, pos, dims, color);
    mBackground.emplace_back(img);
    return img;
}