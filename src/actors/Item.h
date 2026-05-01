#ifndef ASTRAL_ITEM_H
#define ASTRAL_ITEM_H

#include <functional>
#include <utility>
#include <string>
#include "Actor.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../components/collider/AABBColliderComponent.h"
#include "../components/RigidBodyComponent.h"

enum Button
{
    X,A,LT,Y,B,RT,LB,RB,None
};

static SDL_GameControllerButton GetSDLButton(Button button) {
    switch (button)
    {
    case Button::X:
        return SDL_CONTROLLER_BUTTON_X;
    case Button::A:
        return SDL_CONTROLLER_BUTTON_A;
    case Button::LT:
        return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    case Button::Y:
        return SDL_CONTROLLER_BUTTON_Y;
    case Button::B:
        return SDL_CONTROLLER_BUTTON_B;
    case Button::RT:
        return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    case Button::LB:
        return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
    case Button::RB:
        return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    default:
        return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

inline Button GetButtonFromSDL(SDL_GameControllerButton sdlButton) {
    switch (sdlButton)
    {
    case SDL_CONTROLLER_BUTTON_X:
        return Button::X;
    case SDL_CONTROLLER_BUTTON_A:
        return Button::A;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return Button::LT; // or LB, context needed
    case SDL_CONTROLLER_BUTTON_Y:
        return Button::Y;
    case SDL_CONTROLLER_BUTTON_B:
        return Button::B;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return Button::RT; // or RB, context needed
    default:
        return Button::None;
    }
}

static std::map<Button, std::pair<int, int>> joystickButtonsSpriteMapping = {
    {Button::X, {0, 4}},
    {Button::A, {5, 9}},
    {Button::LT, {10, 14}},
    {Button::Y, {15, 19}}, // this is only four (originally in the assetpack)
    {Button::RT, {20, 23}},
    {Button::B, {24, 27}},
    {Button::LB, {28, 32}},
    {Button::RB, {33, 36}}, // this is only four (originally in the assetpack)
};

class Item : public Actor
{
using PickHandler = std::function<void(Item &)>;

public:
    //default constructor
    Item() = default;

    static Item* CreateNevascaItem(Game *game, const Vector2& position, PickHandler onPickCallback = nullptr);
    
private:
    Item(
        Game *game, 
        const Vector2& position, 
        std::string texturePath,
        std::string dataPath,
        int bbWidth, int bbHeight,
        PickHandler onPickCallback = nullptr,
        Button button = Button::None,
        int animationStartIdx=0, int animationEndIdx=0,
        float animFPS=10.f
    );
    
    bool mIsPicked, mIsPickable;
    PickHandler mOnPickCallback;

    DrawAnimatedComponent *mDrawComponent, *mButtonDrawComponent;
    AABBColliderComponent *mColliderComponent;
    RigidBodyComponent *mRigidBodyComponent;

    void OnUpdate(float deltaTime) override;
    void OnProcessInput(const Uint8* keyState, const std::vector<SDL_Event>& events) override;
    void Kill() override;

    Button mButton;
};

#endif