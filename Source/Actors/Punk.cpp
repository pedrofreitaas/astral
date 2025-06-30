//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Punk.h"
#include "Block.h"
#include "Projectile.h"
#include "ProjectileEffect.h"
#include "../Game.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"
#include "../Components/DrawComponents/DrawPolygonComponent.h"

Punk::Punk(Game* game, const float forwardSpeed, const float jumpSpeed)
        : Actor(game)
        , mIsRunning(false)
        , mIsOnPole(false)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mJumpSpeed(jumpSpeed)
        , mPoleSlideTimer(0.0f)
        , mIsShooting(false)
        , mFireCooldown(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f, false);
    mColliderComponent = new AABBColliderComponent(this, 14, 20, 18, 28,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                              "../Assets/Sprites/Punk/texture.png",
                                              "../Assets/Sprites/Punk/texture.json",
                                              static_cast<int>(DrawLayerPosition::Player)+1
                                            );

    mDrawComponent->AddAnimation("dying", {13,14,15,16,17,18});
    mDrawComponent->AddAnimation("idle", {0,1,2,3});
    mDrawComponent->AddAnimation("run", {4,5,6,7,8,9});
    mDrawComponent->AddAnimation("jump", {10,11,12,13});
    mDrawComponent->AddAnimation("shooting", {3});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);

    mArm = new Actor(mGame);
    mArmDraw = new DrawSpriteComponent(mArm, "../Assets/Sprites/Punk/arm_gun.png", 18, 28, 200);
    mArmDraw->SetPivot(Vector2(0.5f, 0.5f));
}

void Punk::OnProcessInput(const uint8_t* state)
{
    //if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    if (mIsDying) return;

    mIsRunning = false;

    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))) {

        Vector2 mouseWorld = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY)) + GetGame()->GetCameraPos();
        ShootAt(mouseWorld);

    } else {
        mIsShooting = false;
        if (state[SDL_SCANCODE_D]) {
            mRigidBodyComponent->ApplyForce(Vector2(mForwardSpeed, 0.0f));
            SetRotation(0.0f);
            mIsRunning = true;
        }
        if (state[SDL_SCANCODE_A]) {
            mRigidBodyComponent->ApplyForce(Vector2(-mForwardSpeed, 0.0f));
            SetRotation(Math::Pi);
            mIsRunning = true;
        }

        if (state[SDL_SCANCODE_W]) {
            mRigidBodyComponent->ApplyForce(Vector2(0.0f, -mForwardSpeed));
            mIsRunning = true;
        }

        if (state[SDL_SCANCODE_S]) {
            mRigidBodyComponent->ApplyForce(Vector2(0.0f, mForwardSpeed));
            mIsRunning = true;
        }
    }
}

void Punk::OnHandleKeyPress(const int key, const bool isPressed)
{
    if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;

    // Jump
    // if (key == SDLK_SPACE && isPressed && mIsOnGround)
    // {
    //     mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed));
    //     mIsOnGround = false;
    //
    //     // Play jump sound
    //     mGame->GetAudio()->PlaySound("Jump.wav");
    // }
}

void Punk::ShootAt(Vector2 targetPos)
{
    mIsShooting = true;
    mIsRunning = false;
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, 0.0f));

    Vector2 center = mColliderComponent->GetMin() + Vector2(mColliderComponent->GetWidth() / 2, mColliderComponent->GetHeight() / 2);
    Vector2 direction = targetPos - center;
    direction.Normalize();

    if (targetPos.x > center.x) {
        SetRotation(0.0f);
        mArmDraw->SetFlip(false);
    } else {
        SetRotation(Math::Pi);
        mArmDraw->SetFlip(true);
    }

    Vector2 shoulderOffset = (GetRotation() == 0.0f) ? Vector2(-2.0f, -20.0f) : Vector2(-13.0f, -20.0f);
    mArm->SetPosition(center + shoulderOffset);
    float angle = atan2f(direction.y, direction.x);
    mArm->SetRotation(angle);

    if (mFireCooldown <= 0.0f) {
        Projectile* projectile = new Projectile(mGame, 5.0f, 1.0f, ColliderLayer::PlayerProjectile);
        Vector2 shotOffset = (GetRotation() == 0.0f) ? Vector2(2.0f, -7.0f) : Vector2(-4.0f, -7.0f);
        projectile->SetPosition(center + shotOffset);
        projectile->GetComponent<RigidBodyComponent>()->ApplyForce(direction * 3000.0f);

        new ProjectileEffect(mGame, center + shotOffset, angle);

        mFireCooldown = 0.5f;
    }
}

void Punk::TakeDamage()
{
    if (mIsDying) return;

    if (mInvincibilityTimer > 0.0f) return;

    mLives--;
    mInvincibilityTimer = 0.25f;

    if (mLives <= 0) {
        Kill();
    } else {
        SDL_Log("PUNK: Took damage. Lives left: %d", mLives);
    }
}

void Punk::MaintainInbound() {
    Vector2 cameraPos = GetGame()->GetCameraPos();
    Vector2 getUpperLeftBorder = mColliderComponent->GetMin();
    Vector2 getBottomRightBorder = mColliderComponent->GetMax();
    Vector2 offset = mColliderComponent->GetOffset();
    int mWidth = mColliderComponent->GetWidth();
    int mHeight = mColliderComponent->GetHeight();
    int maxXBoundary = cameraPos.x + GetGame()->GetWindowWidth();
    int maxYBoundary = cameraPos.y + GetGame()->GetWindowHeight();

    if (getUpperLeftBorder.x < 0) {
        SetPosition(Vector2(-offset.x, GetPosition().y));
    }
    else if (getBottomRightBorder.x > maxXBoundary) {
        SetPosition(Vector2(maxXBoundary-mWidth-offset.x, GetPosition().y));
    }

    if (getUpperLeftBorder.y < 0) {
        SetPosition(Vector2(GetPosition().x, -offset.y));
    }
    else if (getBottomRightBorder.y > maxYBoundary) {
        SetPosition(Vector2(GetPosition().x, maxYBoundary-mHeight-offset.y));
    }
}

void Punk::OnUpdate(float deltaTime)
{
    // Check if Punk is off the ground
    // if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y != 0) {
    //     mIsOnGround = false;
    // }
    //
    // // Limit Punk's position to the camera view
    // mPosition.x = Math::Max(mPosition.x, mGame->GetCameraPos().x);
    //
    // // Kill Punk if he falls below the screen
    // if (mGame->GetGamePlayState() == Game::GamePlayState::Playing && mPosition.y > mGame->GetWindowHeight())
    // {
    //     Kill();
    // }

    // if (mIsOnPole)
    // {
    //     // If Punk is on the pole, update the pole slide timer
    //     mPoleSlideTimer -= deltaTime;
    //     if (mPoleSlideTimer <= 0.0f)
    //     {
    //         mRigidBodyComponent->SetApplyGravity(true);
    //         mRigidBodyComponent->SetApplyFriction(false);
    //         mRigidBodyComponent->SetVelocity(Vector2::UnitX * 100.0f);
    //         mGame->SetGamePlayState(Game::GamePlayState::Leaving);
    //
    //         // Play win sound
    //         mGame->GetAudio()->PlaySound("StageClear.wav");
    //         mIsOnPole = false;
    //         mIsRunning = true;
    //     }
    // }

    // If Punk is leaving the level, kill him if he enters the castle
    // TESTE PARA ELE MUDAR DE NIVEL.
    //ENTÃO QUANDO O MAPA ESTIVER PRONTO É SO COLOCAR A POSIÇÃO DO
    //PORTAL AQUI QUE ELE MUDA DE NIVEL
    // const float castleDoorPos = 500;
    //
    // if (mPosition.x >= castleDoorPos)
    // {
    //     // Stop Punk and set the game scene to Level 2
    //     mState = ActorState::Destroy;
    //     mGame->SetGameScene(Game::GameScene::Level2, 3.5f);
    //
    //     return;
    // }

    mFireCooldown -= deltaTime;
    if (mIsShooting)
        mArmDraw->SetIsVisible(true);
    else
        mArmDraw->SetIsVisible(false);

    if (mInvincibilityTimer > 0.0f)
        mInvincibilityTimer -= deltaTime;

    MaintainInbound();
    ManageAnimations();
    if (!mIsDying) return;

    // mDeathTimer -= deltaTime;
    //
    // if (mDeathTimer > 0.0f) return;

    mColliderComponent->SetEnabled(false);
    mRigidBodyComponent->SetEnabled(false);
    mDrawComponent->SetEnabled(false);

    mGame->Quit();
}

void Punk::ManageAnimations()
{
    if(mIsDying)
    {
        mDrawComponent->SetAnimation("dying");
    }
    else if (mIsShooting) {
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
    mGame->GetAudio()->PlaySound("Dead.wav");

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

void Punk::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        TakeDamage();
        return;
    }

    if (other->GetLayer() == ColliderLayer::EnemyProjectile) {
        TakeDamage();
        other->GetOwner()->SetState(ActorState::Destroy);
        return;
    }

    else if (other->GetLayer() == ColliderLayer::Portal) {
        SDL_Log("PUNK: Entering portal");
        return;
    }
}

void Punk::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        //other->GetOwner()->Kill();
        TakeDamage();
        //mRigidBodyComponent->SetVelocity(Vector2(mRigidBodyComponent->GetVelocity().x, mJumpSpeed / 2.5f));

        // Play jump sound
        //mGame->GetAudio()->PlaySound("Stomp.wav");
    }
    if (other->GetLayer() == ColliderLayer::EnemyProjectile) {
        TakeDamage();
        other->GetOwner()->SetState(ActorState::Destroy);
    }

    else if (other->GetLayer() == ColliderLayer::Bricks && minOverlap < 0) {
        Block *block = static_cast<Block*>(other->GetOwner());
        block->OnColision();
    }
    // else if (other->GetLayer() == ColliderLayer::Blocks)
    // {
        // if (!mIsOnGround)
        // {
        //     // Play bump sound
        //     mGame->GetAudio()->PlaySound("Bump.wav");
        //
        //     // Cast actor to Block to call OnBump
        //     Block* block = static_cast<Block*>(other->GetOwner());
        //     block->OnBump();
        // }
    // }
}