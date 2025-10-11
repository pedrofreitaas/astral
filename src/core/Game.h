// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>
#include <unordered_map>
#include "AudioSystem.h"
#include "../libs/Math.h"
#include "../libs/Json.h"
#include "../ui/UICursor.h"
#include "Map.h"
#include "Tileset.h"
#include "./Cutscene.h"

class Game
{
public:
    static const int LEVEL_WIDTH = 60;
    static const int LEVEL_HEIGHT = 60;
    static const int TILE_SIZE = 32;
    static const int SPAWN_DISTANCE = 700;
    static const int TRANSITION_TIME = 1;

    enum class GameScene
    {
        MainMenu,
        Level1,
    };

    enum class SceneManagerState
    {
        None,
        Entering,
        Active,
        Exiting
    };

    enum class GamePlayState
    {
        Playing,
        Paused,
        GameOver,
        LevelComplete,
        Leaving,
        Dialogue,
        PlayingCutscene
    };

    Game(int windowWidth, int windowHeight);

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);
    void ProcessInputActors();
    void HandleKeyPressActors(const int key, const bool isPressed);

    // Level functions
    void LoadMainMenu();

    std::vector<Actor *> GetNearbyActors(const Vector2& position, const int range = 1);
    std::vector<class AABBColliderComponent *> GetNearbyColliders(const Vector2& position, const int range = 2);

    void Reinsert(Actor* actor);

    // Camera functions
    Vector2& GetCameraPos() { return mCameraPos; };
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };

    // Audio functions
    class AudioSystem* GetAudio() { return mAudio; }

    // UI functions
    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen);}
    void PopUI() { mUIStack.pop_back();}
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    // Window functions
    int GetWindowWidth() const { return mWindowWidth; }
    int GetWindowHeight() const { return mWindowHeight; }

    int GetRealWindowWidth() const { return mRealWindowWidth; }
    int GetRealWindowHeight() const { return mRealWindowHeight; }

    // Loading functions
    class UIFont* LoadFont(const std::string& fileName);
    SDL_Texture* LoadTexture(const std::string& texturePath);

    void SetGameScene(GameScene scene, float transitionTime = .0f);
    void ResetGameScene(float transitionTime = .0f);
    void UnloadScene();

    void SetBackgroundImage(
        const std::string& imagePath, 
        const Vector2 &position = Vector2::Zero, 
        const Vector2& size = Vector2::Zero,
        bool isCameraWise = true
    );
    void TogglePause();

    // Game-specific
    const class Zoe* GetZoe() { return mZoe; }

    void SetGamePlayState(GamePlayState state) { mGamePlayState = state; }
    GamePlayState GetGamePlayState() const { return mGamePlayState; }
    GameScene GetGameScene() const { return mGameScene; }
    SDL_Window* GetWindow() { return mWindow; }
    SDL_Renderer* GetRenderer() const { return mRenderer; }

    int GetGameTotalActors();
    Vector2 GetLogicalMousePos() const;

    void AddCutscene(const std::string& name, std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback = nullptr);
    void StartCutscene(const std::string& name);
    void PauseCutscene();
    void ResetCutscenes();

    class DialogueSystem* GetDialogueSystem() { return mDialogueSystem; }
    void SetStar(class Star* star) { mStar = star; };
    class Star* GetStar() { return mStar; };

private:
    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // Scene Manager
    void UpdateSceneManager(float deltaTime);
    void ChangeScene();
    void SetMap(const std::string& path);
    void LoadFirstLevel();

    bool mDebugMode;

    SceneManagerState mSceneManagerState;
    float mSceneManagerTimer;
    
    // Spatial Hashing for collision detection
    class SpatialHashing* mSpatialHashing;

    // All the UI elements
    std::vector<class UIScreen*> mUIStack;
    std::unordered_map<std::string, class UIFont*> mFonts;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    AudioSystem* mAudio;

    // Window properties
    int mWindowWidth, mWindowHeight;
    int mRealWindowWidth, mRealWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track actors state
    bool mIsRunning;
    GamePlayState mGamePlayState;

    // Track level state
    GameScene mGameScene;
    GameScene mNextScene;

    // Background and camera
    Vector3 mBackgroundColor;
    Vector3 mModColor;
    Vector2 mCameraPos;

    // Game-specific
    class Zoe *mZoe;
    class Star *mStar;
    class Enemy *mEnemy;
    class HUD *mHUD;
    SoundHandle mMusicHandle;

    SDL_Texture *mBackgroundTexture;
    Vector2 mBackgroundSize;
    Vector2 mBackgroundPosition;
    bool mBackgroundIsCameraWise;

    class Map* mMap;

    std::map<std::string, Cutscene*> mCutscenes;
    Cutscene* mCurrentCutscene;

    class DialogueSystem* mDialogueSystem;
};

