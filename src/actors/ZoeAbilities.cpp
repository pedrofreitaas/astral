#include "./Zoe.h"

void Zoe::OnJumpPressed()
{
    if (!IsSDLButtonBlocked(Zoe::JUMP_BUTTON))
    {
        mIsTryingToJump = true;
    }
}

void Zoe::OnJumpReleased()
{
    mIsTryingToJump = false;
}

void Zoe::OnAttackPressed()
{
    if (!IsSDLButtonBlocked(Zoe::HIT_BUTTON))
    {
        mIsTryingToHit = true;
    }
}

void Zoe::OnAttackReleased()
{
    mIsTryingToHit = false;
}

void Zoe::OnDodgePressed()
{
    if (!IsSDLButtonBlocked(Zoe::DODGE_BUTTON))
    {
        mIsTryingToDodge = true;
    }
}

void Zoe::OnDodgeReleased()
{
    mIsTryingToDodge = false;
}

void Zoe::OnVentaniaPressed()
{
    if (!IsSDLButtonBlocked(Zoe::VENTANIA_BUTTON))
    {
        mTryingToTriggerVentania = true;
    }
}

void Zoe::OnVentaniaReleased()
{
    mTryingToTriggerVentania = false;
}

void Zoe::OnFireballPressed()
{
    if (!IsSDLButtonBlocked(Zoe::FIREBALL_BUTTON))
    {
        mTryingToFireFireball = true;
    }
}

void Zoe::OnFireballReleased()
{
    mTryingToFireFireball = false;
}

void Zoe::OnNevascaPressed()
{
    if (!IsSDLButtonBlocked(Zoe::NEVASCA_BUTTON))
    {
        mIsTryingToNevasca = true;
    }
}

void Zoe::OnNevascaReleased()
{
    mIsTryingToNevasca = false;
}

void Zoe::CheckAbilitiesKeys(const std::vector<SDL_Event>& events, SDL_GameController* controller)
{
    // these abilities should be reset every frame - to avoid keeping them pressed for trigger
    mIsTryingToJump = false;
    mIsTryingToHit = false;
    mTryingToFireFireball = false;
    mIsTryingToDodge = false;
    // mTryingToTriggerVentania = false; - this should be per frame, but there is BUG - TODO
    // mIsTryingToNevasca = false; - isnt per frame!

    if (mAbilitiesLocked || controller == nullptr) return;

    for (const auto &event : events)
    {
        switch (event.type)
        {
        case SDL_CONTROLLERBUTTONDOWN:
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
            {
                OnJumpPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
            {
                OnAttackPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
            {
                OnFireballPressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
            {
                OnDodgePressed();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
            {
                OnVentaniaPressed();
            }

            break;

        case SDL_CONTROLLERBUTTONUP:
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
            {
                OnJumpReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
            {
                OnAttackReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
            {
                OnFireballReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B)
            {
                OnDodgeReleased();
            }

            else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
            {
                OnVentaniaReleased();
            }

            break;
        }
    }

    if (
        !IsSDLButtonBlocked(Zoe::NEVASCA_BUTTON) && 
        SDL_GameControllerGetAxis(controller, Zoe::NEVASCA_AXIS) > 0
    )
    {
        OnNevascaPressed();
    }
    else
    {
        OnNevascaReleased();
    }
}