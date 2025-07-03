//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Punk.h"
#include "Block.h"
#include "Projectile.h"
#include "ProjectileEffect.h"
#include "../Game.h"
#include "Portal.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

Punk::Punk(Game *game, const float forwardSpeed, const float jumpSpeed)
    : Actor(game), mIsRunning(false), mIsOnPole(false), mIsDying(false), mForwardSpeed(forwardSpeed), mJumpSpeed(jumpSpeed), mPoleSlideTimer(0.0f), mIsShooting(false), mFireCooldown(0.0f), mFoundKey(false), mDeathTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);
    mColliderComponent = new AABBColliderComponent(this, 14, 20, 18, 28,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                               "../Assets/Sprites/Punk/texture.png",
                                               "../Assets/Sprites/Punk/texture.json",
                                               static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("dying", {13, 14, 15, 16, 17, 18});
    mDrawComponent->AddAnimation("idle", {0, 1, 2, 3});
    mDrawComponent->AddAnimation("run", {4, 5, 6, 7, 8, 9});
    mDrawComponent->AddAnimation("jump", {10, 11, 12, 13});
    mDrawComponent->AddAnimation("shooting", {3});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);

    mArm = new PunkArm(mGame, this);
}

void Punk::OnProcessInput(const uint8_t *state)
{
    // if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    if (mIsDying)
        return;

    mIsRunning = false;

    if (mArm->mIsShooting) return;

    if (state[SDL_SCANCODE_D])
    {
        mRigidBodyComponent->ApplyForce(Vector2(mForwardSpeed, 0.0f));
        SetRotation(0.0f);
        mIsRunning = true;
    }
    if (state[SDL_SCANCODE_A])
    {
        mRigidBodyComponent->ApplyForce(Vector2(-mForwardSpeed, 0.0f));
        SetRotation(Math::Pi);
        mIsRunning = true;
    }

    if (state[SDL_SCANCODE_W])
    {
        mRigidBodyComponent->ApplyForce(Vector2(0.0f, -mForwardSpeed));
        mIsRunning = true;
    }

    if (state[SDL_SCANCODE_S])
    {
        mRigidBodyComponent->ApplyForce(Vector2(0.0f, mForwardSpeed));
        mIsRunning = true;
    }
}

void Punk::OnHandleKeyPress(const int key, const bool isPressed)
{
}

void Punk::TakeDamage()
{
    if (mIsDying)
        return;

    if (mInvincibilityTimer > 0.0f)
        return;

    mLives--;
    mInvincibilityTimer = 0.25f;

    if (mLives <= 0)
    {
        mIsDying = true;
        mDrawComponent->SetAnimFPS(8.0f);
        mDeathTimer = DEATH_TIMER;
    }
}

void Punk::MaintainInbound()
{
    Vector2 cameraPos = GetGame()->GetCameraPos();
    Vector2 getUpperLeftBorder = mColliderComponent->GetMin();
    Vector2 getBottomRightBorder = mColliderComponent->GetMax();
    Vector2 offset = mColliderComponent->GetOffset();
    int mWidth = mColliderComponent->GetWidth();
    int mHeight = mColliderComponent->GetHeight();
    int maxXBoundary = cameraPos.x + GetGame()->GetWindowWidth();
    int maxYBoundary = cameraPos.y + GetGame()->GetWindowHeight();

    if (getUpperLeftBorder.x < 0)
    {
        SetPosition(Vector2(-offset.x, GetPosition().y));
    }
    else if (getBottomRightBorder.x > maxXBoundary)
    {
        SetPosition(Vector2(maxXBoundary - mWidth - offset.x, GetPosition().y));
    }

    if (getUpperLeftBorder.y < 0)
    {
        SetPosition(Vector2(GetPosition().x, -offset.y));
    }
    else if (getBottomRightBorder.y > maxYBoundary)
    {
        SetPosition(Vector2(GetPosition().x, maxYBoundary - mHeight - offset.y));
    }
}

void Punk::OnUpdate(float deltaTime)
{
    MaintainInbound();
    ManageAnimations();

    if (mIsDying)
    {
        mDeathTimer -= deltaTime;

        if (mDeathTimer <= 0)
        {
            mGame->Quit();
        }
        return;
    }

    if (mInvincibilityTimer > 0.0f)
        mInvincibilityTimer -= deltaTime;
}

void Punk::ManageAnimations()
{
    if (mIsDying)
    {
        mDrawComponent->SetAnimation("dying");
    }
    else if (mArm->mIsShooting)
    {
        mDrawComponent->SetAnimation("shooting");
    }
    else if (mIsRunning)
    {
        mDrawComponent->SetAnimation("run");
    }
    else if (!mIsRunning)
    {
        mDrawComponent->SetAnimation("idle");
    }
}

void Punk::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("dying");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    // mGame->GetAudio()->PlaySound("Dead.wav");

    mGame->ResetGameScene(3.5f); // Reset the game scene after 3 seconds
}

void Punk::Win(AABBColliderComponent *poleCollider)
{
    mDrawComponent->SetAnimation("win");
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);

    // Set Punk velocity to go down
    mRigidBodyComponent->SetVelocity(Vector2::UnitY * 100.0f); // 100 pixels per second
    mRigidBodyComponent->SetApplyGravity(false);

    // Disable collider
    poleCollider->SetEnabled(false);

    // Adjust Punk x position to grab the pole
    mPosition.Set(poleCollider->GetOwner()->GetPosition().x + Game::TILE_SIZE / 4.0f, mPosition.y);

    // Stop music
    mGame->GetAudio()->StopAllSounds();

    mPoleSlideTimer = POLE_SLIDE_TIME; // Start the pole slide timer
}

void Punk::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::EnemyProjectile)
    {
        TakeDamage();
        other->GetOwner()->SetState(ActorState::Destroy);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level1)
    {
        mGame->SetGameScene(Game::GameScene::Level2, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level2)
    {
        mGame->SetGameScene(Game::GameScene::FinalScene, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Item)
    {
        other->GetOwner()->OnCollision();
    }
}

void Punk::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::EnemyProjectile)
    {
        TakeDamage();
        other->GetOwner()->SetState(ActorState::Destroy);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level1)
    {
        mGame->SetGameScene(Game::GameScene::Level2, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level2)
    {
        mGame->SetGameScene(Game::GameScene::FinalScene, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Item)
    {
        other->GetOwner()->OnCollision();
    }
}

void Punk::FindKey()
{
    mFoundKey = true;
    const auto &portal = new Portal(mGame);
    portal->SetPosition(Vector2(243.0f, 620.0f));
    mGame->GetAudio()->PlaySound("KeyPick.wav");
}