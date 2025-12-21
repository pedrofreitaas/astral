#pragma once
#include <SDL.h>
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "Ventania.h"
#include "../components/collider/AABBColliderComponent.h"
#include "Tile.h"
#include "ZoeFireball.h"
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

const SDL_Rect DEFAULT_BB = SDL_Rect({22, 18, 20, 27});
const SDL_Rect DODGE_BB = SDL_Rect({25, 25, 13, 15});

class Zoe : public Actor
{
    std::vector<ColliderLayer> IGNORED_LAYERS_DODGE = {
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile,
        ColliderLayer::PlayerAttack
    };

    std::vector<ColliderLayer> IGNORED_LAYERS_DEFAULT = {
        ColliderLayer::PlayerAttack
    };
    
    float FIREBALL_SPEED = 20000.f;
    float FIREBALL_COOLDOWN = 4.f;
    
    float VETANIA_SPEED = 20000.f;
    float VETANIA_COOLDOWN = 0.75f;

    float DODGE_COOLDOWN = 1.0f;

    float DEFAULT_KNOCKBACK_FORCE = 2400.f;
    
    SDL_Scancode FIREBALL_KEY = SDL_SCANCODE_Q;
    SDL_Scancode VENTANIA_KEY = SDL_SCANCODE_E;
    SDL_Scancode JUMP_KEY = SDL_SCANCODE_SPACE;
    SDL_Scancode HIT_KEY = SDL_SCANCODE_R;
    SDL_Scancode DODGE_KEY = SDL_SCANCODE_LSHIFT;
    
    SDL_GameControllerButton DODGE_BUTTON = SDL_CONTROLLER_BUTTON_B;
    SDL_GameControllerButton FIREBALL_BUTTON = SDL_CONTROLLER_BUTTON_Y;
    SDL_GameControllerButton VENTANIA_BUTTON = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    SDL_GameControllerButton JUMP_BUTTON = SDL_CONTROLLER_BUTTON_A;
    SDL_GameControllerButton HIT_BUTTON = SDL_CONTROLLER_BUTTON_X;

public:
    explicit Zoe(Game* game, float forwardSpeed, const Vector2 &center);
    ~Zoe();

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnHandleKeyPress(const int key, const bool isPressed) override;

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;
    void ManageState();
    void SetInvincible(bool invincible) { mInvincible = invincible; }
    void AnimationEndCallback(std::string animationName);
    
    Vector2 GetFireballOffset() {
        if (GetRotation() == 0.f) {
            return Vector2(50.f,35.f);
        }
        else {
            return Vector2(12.f,36.f);
        }
    }

    void TakeDamage(const Vector2 &knockback = Vector2(0.f, 0.f)) override;

    bool IsMovementLocked() const;
    void SetMovementLocked(bool locked);

    bool IsAbilitiesLocked() const;
    void SetAbilitiesLocked(bool locked);

    bool CheckFireballOnCooldown() {
        return mFireballCooldownTimer && 
               mTimerComponent->checkTimerRemaining(mFireballCooldownTimer) > 0.f;
    }

    float GetFireballCooldownProgress() {
        if (mFireballCooldownTimer) {
            return mTimerComponent->checkTimerRemaining(mFireballCooldownTimer) / FIREBALL_COOLDOWN;
        }
        return 1.f;
    }

    void TakeSithAttack1(const float minOverlap, AABBColliderComponent *other);
    void TakeSithAttack2(const float minOverlap, AABBColliderComponent *other);

private:
    float mForwardSpeed;

    class TimerComponent *mTimerComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    void ManageAnimations();

    Vector2 mInputMovementDir;
    bool mIsTryingToHit, mIsTryingToDodge, mIsTryingToJump;
    Timer* mDodgeCooldownTimer;

    void FireFireball();
    bool mTryingToFireFireball;
    Timer* mFireballCooldownTimer;

    void TriggerVentania();
    void SetVentaniaOnCooldown(bool onCooldown) { mIsVentaniaOnCooldown = onCooldown; }
    bool mIsVentaniaOnCooldown, mTryingToTriggerVentania;

    Collider *mAttackCollider, *mAerialAttackCollider;

    Timer *mCoyoteTimer;

    SoundHandle mDamageSoundHandle;

    bool mAbilitiesLocked, mMovementLocked;

    void Move(float modifier=1.f);
    bool CheckJump();
    bool CheckDodge(); 
    bool CheckHit();
    void DodgeEnd();
    void EndAerialAttack();

    int IsPressingAgainstWall();
};
