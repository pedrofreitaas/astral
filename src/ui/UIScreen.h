// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <list>

#include "../libs/Math.h"
#include "UIText.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UICursor.h"

class UIScreen
{
public:
    // Tracks if the UI is active or closing
    enum class UIState
    {
        Active,
        Closing
    };

	UIScreen(class Game* game, const std::string& fontName);
	virtual ~UIScreen();

	// UIScreen subclasses can override these
	virtual void Update(float deltaTime);
	virtual void Draw(class SDL_Renderer *renderer);
	virtual void HandleKeyPress(int key);
	virtual void HandleMouseClick(int button, int x, int y);

    // Set state to closing
	void Close();

    // Get state of UI screen
	UIState GetState() const { return mState; }

    // Game getter
    class Game* GetGame() { return mGame; }

    // Add a button to this screen
	UIButton* AddButton(const std::string& name, const Vector2& pos, const Vector2& dims, std::function<void()> onClick, const Vector2& textSize=Vector2(140.f, 20.0f));
    UIText* AddText(const std::string& name, const Vector2& pos, const Vector2& dims, const int pointSize = 40, const int unsigned wrapLength = 1024, const Vector3& color = Vector3(0.0f,0.0f,95.0f));
    UIImage* AddImage(const std::string& imagePath, const Vector2& pos, const Vector2& dims, const Vector3& color = Color::White);
	UIImage* AddCursor(const std::string& imagePath, const Vector2& pos, const Vector2& dims, const Vector3& color = Color::White);
	UIImage* AddBackground(const std::string& imagePath, const Vector2& pos, const Vector2& dims, const Vector3& color = Color::White);

protected:
    // Sets the mouse mode to relative or not
	class Game* mGame;
	class UIFont* mFont;

	// Configure positions
	Vector2 mPos;
	Vector2 mSize;

	// State
	UIState mState;

	// List of buttons, texts, and images
    int mSelectedButtonIndex;
	std::vector<UIButton *> mButtons;
    std::vector<UIText *> mTexts;
    std::vector<UIImage *> mImages;
    std::vector<UICursor *> mCursors;
    std::vector<UIImage *> mBackground;
};
