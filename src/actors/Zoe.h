#pragma once
#include "Actor.h"
#include "Projectile.h"
#include "Collider.h"
#include "../components/collider/AABBColliderComponent.h"
#include <SDL.h>

class Ventania : public Actor
{
public:
    Ventania(Game* game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed = .0f);

private:    
    class DrawAnimatedComponent *mDrawAnimatedComponent;
    void AnimationEndCallback(std::string animationName);
};

class Fireball : public Projectile
{
    int MAX_RICOCHETS = 3;

public:
    Fireball(
        class Game* game, Vector2 position, 
        Vector2 direction, float speed, Actor* shooter
    );

private:
    void ManageAnimations();
    void AnimationEndCallback(std::string animationName);

    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;

    void Kill() override;

    int mRicochetsCount;
};

class Zoe : public Actor
{
    std::vector<ColliderLayer> IGNORED_LAYERS_DODGE = {
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile
    };

    std::vector<ColliderLayer> IGNORED_LAYERS_DEFAULT = {
    };
    
    float FIREBALL_SPEED = 20000.f;
    float FIREBALL_COOLDOWN = 4.f;
    
    float VETANIA_SPEED = 20000.f;
    float VETANIA_COOLDOWN = 0.75f;
    
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
            return Vector2(50.f,53.f);
        }
        else {
            return Vector2(12.f,52.f);
        }
    }

    void TakeDamage(const Vector2 &knockback = Vector2(0.f, 0.f)) override;

    bool IsMovementLocked() const { return mMovementLocked; }
    void SetMovementLocked(bool locked) { mMovementLocked = locked; }

    bool IsAbilitiesLocked() const { return mAbilitiesLocked; }
    void SetAbilitiesLocked(bool locked) { mAbilitiesLocked = locked; }

private:
    float mForwardSpeed;

    class TimerComponent *mTimerComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class DrawAnimatedComponent* mDrawComponent;
    class AABBColliderComponent* mColliderComponent;

    void ManageAnimations();

    Vector2 mInputMovementDir;
    bool mIsTryingToHit, mIsTryingToDodge, mIsTryingToJump;

    void FireFireball();
    void SetFireballOnCooldown(bool onCooldown) { mIsFireballOnCooldown = onCooldown; }
    bool mIsFireballOnCooldown, mTryingToFireFireball;

    void TriggerVentania();
    void SetVentaniaOnCooldown(bool onCooldown) { mIsVentaniaOnCooldown = onCooldown; }
    bool mIsVentaniaOnCooldown, mTryingToTriggerVentania;

    Collider *mAttackCollider;

    bool mAbilitiesLocked, mMovementLocked;
};
