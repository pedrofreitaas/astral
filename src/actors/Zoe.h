#pragma once
#include <SDL.h>
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "Ventania.h"
#include "../components/collider/AABBColliderComponent.h"
#include "Tile.h"
#include "ZoeFireball.h"
#include "Nevasca.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"
#include "../components/TimerComponent.h"
#include "./Collider.h"
#include "./traps/Spikes.h"
#include "./traps/Spear.h"
#include "./traps/Shuriken.h"
#include "../libs/Math.h"
#include "enemies/Sith.h"
#include "enemies/Quasar.h"
#include "../core/Checkpoint.h"
#include "./Item.h"

const SDL_Rect DEFAULT_BB = SDL_Rect({22, 18, 20, 27});
const SDL_Rect DODGE_BB = SDL_Rect({25, 25, 13, 15});

class Zoe : public Actor
{
    friend class FireNevascaStep;
    friend class Game;
    friend class Item;
    friend class LaunchFireballStep;
    friend class JumpStep;

    std::vector<ColliderLayer> IGNORED_LAYERS_DODGE = {
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile,
        ColliderLayer::PlayerAttack
    };

    std::vector<ColliderLayer> IGNORED_LAYERS_DEFAULT = {
        ColliderLayer::PlayerAttack,
        ColliderLayer::Nevasca
    };

    std::map<Button, bool> mButtonBlocked = {
        {Button::X, false},
        {Button::A, false},
        {Button::LT, false},
        {Button::Y, false},
        {Button::B, false},
        {Button::RT, false},
        {Button::LB, false},
        {Button::RB, false}
    };

    int mDeaths;

    static constexpr SDL_GameControllerButton DODGE_BUTTON = SDL_CONTROLLER_BUTTON_B;
    static constexpr SDL_GameControllerButton FIREBALL_BUTTON = SDL_CONTROLLER_BUTTON_Y;
    static constexpr SDL_GameControllerButton VENTANIA_BUTTON = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    static constexpr SDL_GameControllerButton JUMP_BUTTON = SDL_CONTROLLER_BUTTON_A;
    static constexpr SDL_GameControllerButton HIT_BUTTON = SDL_CONTROLLER_BUTTON_X;
    static constexpr SDL_GameControllerAxis NEVASCA_AXIS = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
    static constexpr SDL_GameControllerButton NEVASCA_BUTTON = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

    Checkpoint *mCurrentCheckpoint;

public:
    explicit Zoe(Game* game, float forwardSpeed, const Vector2 &center);
    ~Zoe();

    void SetCheckpoint(const Vector2 &position);
    Checkpoint* GetCurrentCheckpoint() const;

    void BlockButton(Button button) { mButtonBlocked[button] = true; }
    void UnblockButton(Button button) { mButtonBlocked[button] = false; }
    bool IsButtonBlocked(Button button) { return mButtonBlocked[button]; }
    bool IsSDLButtonBlocked(SDL_GameControllerButton sdlButton) { return mButtonBlocked[GetButtonFromSDL(sdlButton)]; }

    void OnProcessInput(const Uint8* keyState, const std::vector<SDL_Event>& events) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnJumpPressed();
    void OnJumpReleased();
    void OnAttackPressed();
    void OnAttackReleased();
    void OnDodgePressed();
    void OnDodgeReleased();
    void OnVentaniaPressed();
    void OnVentaniaReleased();
    void OnFireballPressed();
    void OnFireballReleased();
    void OnNevascaPressed();
    void OnNevascaReleased();
    void CheckAbilitiesKeys(const std::vector<SDL_Event>& events, SDL_GameController* controller);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    void ManageState();
    void AnimationEndCallback(std::string animationName);
    
    Vector2 GetFireballOffset() {
        if (GetRotation() == 0.f) {
            return Vector2(50.f,35.f);
        }
        else {
            return Vector2(12.f,36.f);
        }
    }

    Vector2 GetNevascaOffset() {
        if (GetRotation() == 0.f) {
            return GetCenter() + Vector2(5.f, -5.f);
        }

        return GetCenter() + Vector2(-2.f, -5.f);
    }

    void TakeDamage() override;

    bool IsMovementLocked() const;
    void SetMovementLocked(bool locked);

    bool IsAbilitiesLocked() const;
    void SetAbilitiesLocked(bool locked);

    bool CheckFireballOnCooldown();

    float GetFireballCooldownProgress();

    void TakeSithAttack1(const float minOverlap, AABBColliderComponent *other);
    void TakeSithAttack2(const float minOverlap, AABBColliderComponent *other);

    float GetMana() const { return mMana; }

    void SetIsFireballAllowed(bool allowed) { mIsFireballAllowed = allowed; }
    void SetIsDodgeAllowed(bool allowed) { mIsDodgeAllowed = allowed; }
    void SetIsVentaniaAllowed(bool allowed) { mIsVentaniaAllowed = allowed; }
    void SetIsNevascaAllowed(bool allowed) { mIsNevascaAllowed = allowed; }

    void TeleportToCheckpoint();

protected:
    void SetMana(float mana);

private:
    float mForwardSpeed, mMana;
    bool mConsumedManaThisFrame;

    bool HasMana(float amount) const { return mMana >= amount; }
    void ConsumeMana(float amount);
    void RegenerateMana();

    Timer* mManaRegenTimerHandle;

    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;
    class TimerComponent *mTimerComponent;

    void ManageAnimations();

    Vector2 mInputMovementDir;
    bool mIsTryingToHit, mIsTryingToDodge, mIsTryingToJump, mIsTryingToNevasca;
    bool mIsFiringNevasca;
    float mNevascaTimer;
    Timer* mDodgeCooldownTimer;
    Timer* mDashGravityDisableTimer;

    bool mIsFireballAllowed, mIsDodgeAllowed, mIsVentaniaAllowed, mIsNevascaAllowed;

    void FireFireball();
    bool mTryingToFireFireball;
    Timer* mFireballCooldownTimer;

    bool CheckVentania();
    void SetLandedAfterVentania(bool landed) { mLandedAfterVentania = landed; }
    bool mLandedAfterVentania, mTryingToTriggerVentania;

    Collider *mAttackCollider, *mAerialAttackCollider;

    Timer *mCoyoteTimer;

    SoundHandle mDamageSoundHandle, mNevascaSoundHandle;

    bool mAbilitiesLocked, mMovementLocked;

    void Move(float modifier=1.f);
    bool CheckJump();
    bool CheckDodge(); 
    bool CheckHit();
    void DodgeEnd();
    bool CheckNevasca();

    int IsPressingAgainstWall();

    void OnDamageCallback();
};
