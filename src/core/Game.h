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
#include <chrono>
#include "AudioSystem.h"
#include "../libs/Math.h"
#include "../libs/Json.h"
#include "../ui/UICursor.h"
#include "Map.h"
#include "Tileset.h"
#include "./Cutscene.h"
#include "Config.h"

template <typename F>
auto TimeCheckerWrapper(F&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
        func();
        SDL_Log("Time: %.3f ms", std::chrono::duration<float, std::milli>(
            std::chrono::high_resolution_clock::now() - start).count());
    } else {
        auto r = func();
        SDL_Log("Time: %.3f ms", std::chrono::duration<float, std::milli>(
            std::chrono::high_resolution_clock::now() - start).count());
        return r;
    }
}

class Game
{
public:
    static const int LEVEL_WIDTH = 60;
    static const int LEVEL_HEIGHT = 60;
    static const int TILE_SIZE = 32;
    static const int TRANSITION_TIME_BETWEEM_SCENES = 2;
    static const bool APPLY_GRAVITY_SCENE_DEFAULT = true;
    const int CHAR_WIDTH = 6;
    const int WORD_HEIGHT = 8;
    const std::string FONT_PATH_INTER = "../assets/Fonts/Inter.ttf";
    const std::string FONT_PATH_SMB = "../assets/Fonts/SMB.ttf";

    enum class GameScene
    {
        MainMenu,
        Bedroom,
        BedroomPortal,
        Level1,
        Tests,
        DeathScreen,
        EndDemo
    };

    enum class CameraCenter
    {
        Zoe,
        Point,
        LogicalWindowSizeCenter
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

    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void UpdateActors(float deltaTime);
    void AddActor(class Actor *actor);
    void RemoveActor(class Actor *actor);
    void ProcessInputActors();
    void HandleKeyPressActors(const int key, const bool isPressed);

    // Level functions
    void LoadMainMenu();
    void LoadBedroom();
    void LoadBedroomPortal();
    void LoadFirstLevel();
    void LoadTestsLevel();
    void LoadDeathScreen();
    void LoadEndDemoScene();

    std::vector<Actor *> GetNearbyActors(const Vector2 &position, const int range = 1);
    std::vector<class AABBColliderComponent *> GetNearbyColliders(const Vector2 &position, const int range = 2);

    void Reinsert(Actor *actor);

    // Camera functions
    Vector2 &GetCameraPos() { return mCameraPos; };
    void SetCameraPos(const Vector2 &position) { mCameraPos = position; };
    bool ActorOnCamera(Actor *actor);

    // Audio functions
    class AudioSystem *GetAudio() { return mAudio; }

    // UI functions
    void PushUI(class UIScreen *screen) { mUIStack.emplace_back(screen); }
    void PopUI() { mUIStack.pop_back(); }
    const std::vector<class UIScreen *> &GetUIStack() { return mUIStack; }

    // Window functions
    int GetWindowWidth() const { return mWindowWidth; }
    int GetWindowHeight() const { return mWindowHeight; }

    int GetRealWindowWidth() const { return mRealWindowWidth; }
    int GetRealWindowHeight() const { return mRealWindowHeight; }

    // Loading functions
    class UIFont *LoadFont(const std::string &fileName);
    SDL_Texture *LoadTexture(const std::string &texturePath);

    void SetGameScene(GameScene scene, float sceneLeftTime = .0f);
    void SetApplyGravityScene(bool applyGravity) {
        mApplyGravityScene = applyGravity;
    };
    bool GetApplyGravityScene() const {
        return mApplyGravityScene;
    };
    void ResetGameScene(float transitionTime = .0f);
    void UnloadScene();

    void SetBackgroundImage(
        const std::string &imagePath,
        const Vector2 &position = Vector2::Zero,
        const Vector2 &size = Vector2::Zero,
        bool isCameraWise = true);
    void TogglePause();

    // Game-specific
    class Zoe *GetZoe() { return mZoe; }
    void SetZoe(class Zoe *zoe);

    GamePlayState GetGamePlayState() const { 
        return mGamePlayState; 
    }

    void SetGamePlayState(GamePlayState state) { 
        mPreviousGameState = GetGamePlayState();
        mGamePlayState = state; 
    }

    void GoBackToPreviousGameState() {
        SetGamePlayState(mPreviousGameState);
    }

    GameScene GetGameScene() const { return mGameScene; }
    SDL_Window *GetWindow() { return mWindow; }
    SDL_Renderer *GetRenderer() const { return mRenderer; }

    int GetGameTotalActors();
    Vector2 GetLogicalMousePos() const;

    void AddCutscene(const std::string &name, std::vector<std::unique_ptr<Step>> steps, std::function<void()> onCompleteCallback = nullptr);
    void StartCutscene(const std::string &name);
    void PauseCutscene();
    void ResetCutscenes();

    class DialogueSystem *GetDialogueSystem() { return mDialogueSystem; }

    void SetStar(class Star *star) { mStar = star; };
    class Star *GetStar() { return mStar; };

    int GetMapWidth();
    int GetMapHeight();

    class SpatialHashing *GetSpatialHashing() { return mSpatialHashing; }

    SDL_GameController* GetController() const { return mController; }

    void AddMustAlwaysUpdateActor(class Actor* actor) {
        mMustAlwaysUpdateActors.push_back(actor);
    }

    void AddEnemy(class Enemy *enemy);
    void RemoveEnemy(class Enemy *enemy);

    std::vector<class Enemy *> GetEnemies(const Vector2 &min, const Vector2 &max);

    Config *GetConfig() { return mConfig; }

    Vector2 getNormalizedControlerPad();

private:
    Config *mConfig;

    void SetCameraCenterToLogicalWindowSizeCenter() {
        mCameraCenter = CameraCenter::LogicalWindowSizeCenter;
        mCameraCenterPos = Vector2(.0f, .0f);
    }
    void SetCameraCenterToZoe() {
        mCameraCenter = CameraCenter::Zoe;
        mCameraCenterPos = Vector2::Zero;
    }
    void SetCameraCenter(const Vector2& newCenter) {
        mCameraCenter = CameraCenter::Point;
        mCameraCenterPos = newCenter;
    }
    void SetMaintainCameraInMap(bool newValue) {
        mMaintainCameraInMap = newValue;
    }

    void ProcessInput();
    void UpdateGame();
    void UpdateCamera();
    void GenerateOutput();

    // Scene Manager
    void UpdateSceneManager(float deltaTime);
    void ChangeScene();
    void SetMap(const std::string &path);

    bool mDebugMode;
    SDL_GameController* mController;

    SceneManagerState mSceneManagerState;
    float mSceneManagerTimer;
    bool mApplyGravityScene;

    // Spatial Hashing for collision detection
    class SpatialHashing *mSpatialHashing;

    // All the UI elements
    std::vector<class UIScreen *> mUIStack;
    std::unordered_map<std::string, class UIFont *> mFonts;

    // SDL stuff
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;
    AudioSystem *mAudio;

    // Window properties
    int mWindowWidth, mWindowHeight;
    int mRealWindowWidth, mRealWindowHeight;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track actors state
    bool mIsRunning;
    GamePlayState mGamePlayState, mPreviousGameState;

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
    std::vector<class Enemy *> mEnemies;
    class HUD *mHUD;
    SoundHandle mMusicHandle;

    SDL_Texture *mBackgroundTexture;
    Vector2 mBackgroundSize;
    Vector2 mBackgroundPosition;
    bool mBackgroundIsCameraWise;

    class Map *mMap;

    std::map<std::string, Cutscene *> mCutscenes;
    Cutscene *mCurrentCutscene;

    class DialogueSystem *mDialogueSystem;

    CameraCenter mCameraCenter;
    Vector2 mCameraCenterPos;
    bool mMaintainCameraInMap;

    void DrawDebugInfo(std::vector<class Actor *> &actorsOnCamera);

    std::vector<class Actor*> mMustAlwaysUpdateActors; //use with caution, can highly impact performance

    Vector2 GetBoxCenter(const Vector2& pos, float boxW, float boxH);

    bool isEnding;
    void EndDemoCheck();
};
