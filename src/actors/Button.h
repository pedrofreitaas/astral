#pragma once
#include <map>
#include <utility>
#include <SDL.h>

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
        return Button::LT;
    case SDL_CONTROLLER_BUTTON_Y:
        return Button::Y;
    case SDL_CONTROLLER_BUTTON_B:
        return Button::B;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return Button::RT;
    default:
        return Button::None;
    }
}

static std::map<Button, std::pair<int, int>> joystickButtonsSpriteMapping = {
    {Button::X, {0, 4}},
    {Button::A, {5, 9}},
    {Button::LT, {10, 14}},
    {Button::Y, {15, 19}},
    {Button::RT, {20, 23}},
    {Button::B, {24, 27}},
    {Button::LB, {28, 32}},
    {Button::RB, {33, 36}},
};
