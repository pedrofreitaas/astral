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
#include "../UIElements/DialogueSystem.h"

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

    mArm = new Actor(mGame);
    mArmDraw = new DrawSpriteComponent(mArm, "../Assets/Sprites/Punk/arm_gun.png", 18, 28, 200);
    mArmDraw->SetPivot(Vector2(0.5f, 0.5f));
}

void Punk::OnProcessInput(const uint8_t *state)
{
    // if(mGame->GetGamePlayState() != Game::GamePlayState::Playing) return;
    if (mIsDying)
        return;

    mIsRunning = false;

    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
    {

        Vector2 mouseWorld = Vector2(static_cast<float>(mouseX), static_cast<float>(mouseY)) + GetGame()->GetCameraPos();
        ShootAt(mouseWorld);
    }
    else
    {
        mIsShooting = false;
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
}

void Punk::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mGame->GetGamePlayState() != Game::GamePlayState::Playing)
        return;

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

    if (targetPos.x > center.x)
    {
        SetRotation(0.0f);
        mArmDraw->SetFlip(false);
    }
    else
    {
        SetRotation(Math::Pi);
        mArmDraw->SetFlip(true);
    }

    Vector2 shoulderOffset = (GetRotation() == 0.0f) ? Vector2(-2.0f, -20.0f) : Vector2(-13.0f, -20.0f);
    mArm->SetPosition(center + shoulderOffset);
    float angle = atan2f(direction.y, direction.x);
    mArm->SetRotation(angle);

    if (mFireCooldown <= 0.0f)
    {
        Projectile *projectile = new Projectile(mGame, 5.0f, 1.0f, ColliderLayer::PlayerProjectile);
        Vector2 shotOffset = (GetRotation() == 0.0f) ? Vector2(2.0f, -7.0f) : Vector2(-4.0f, -7.0f);
        projectile->SetPosition(center + shotOffset);
        projectile->mPreviousPosition = center + shotOffset;
        projectile->GetComponent<RigidBodyComponent>()->ApplyForce(direction * 3000.0f);

        new ProjectileEffect(mGame, center + shotOffset, angle);

        mFireCooldown = 0.5f;
    }
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
    if (mGame->GetGamePlayState() == Game::GamePlayState::Dialogue)
    {
        mDrawComponent->SetAnimation("idle");
        return;
    }

    MaintainInbound();
    ManageAnimations();

    if (mIsDying) {
        mDeathTimer-=deltaTime;

        if (mDeathTimer <= 0) {
            mGame->Quit();
        }
        return;
    }
    
    mFireCooldown -= deltaTime;
    if (mIsShooting)
        mArmDraw->SetIsVisible(true);
    else
        mArmDraw->SetIsVisible(false);

    if (mInvincibilityTimer > 0.0f)
        mInvincibilityTimer -= deltaTime;
}

void Punk::ManageAnimations()
{
    if (mIsDying)
    {
        mDrawComponent->SetAnimation("dying");
    }
    else if (mIsShooting)
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
    //mGame->GetAudio()->PlaySound("Dead.wav");

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

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level1) {
        mGame->SetGameScene(Game::GameScene::Level2, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level2) {
        mGame->SetGameScene(Game::GameScene::Ending_GoHome, .25f);
        other->SetEnabled(false);
        return;
    }
    if (other->GetLayer() == ColliderLayer::Portal2 && mGame->GetGameScene() == Game::GameScene::Level2) {
        mGame->SetGameScene(Game::GameScene::Ending_Stay, .25f);
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

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level1) {
        mGame->SetGameScene(Game::GameScene::Level2, .25f);
        other->SetEnabled(false);
        return;
    }

    if (other->GetLayer() == ColliderLayer::Portal && mGame->GetGameScene() == Game::GameScene::Level2) {
        mGame->SetGameScene(Game::GameScene::Ending_GoHome, .25f);
        other->SetEnabled(false);
        return;
    }
    if (other->GetLayer() == ColliderLayer::Portal2 && mGame->GetGameScene() == Game::GameScene::Level2) {
        mGame->SetGameScene(Game::GameScene::Ending_Stay, .25f);
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
    mGame->GetAudio()->PlaySound("KeyPick.wav");

    if (mGame->GetGameScene() == Game::GameScene::Level1) {
        DialogueSystem::Get()->StartDialogue(
        {
            "Punk: Uma chave! Agora, o que sera que ela abre?",
                "Punk: Melhor eu dar uma olhada ao redor."
        },
    [this]() {
        mGame->SetGamePlayState(Game::GamePlayState::Playing);
    }
    );
        const auto &portal = new Portal(mGame);
        portal->SetPosition(Vector2(622.0f, 210.0f));
    }
    else {
        DialogueSystem::Get()->StartDialogue(
        {
            "Voz: Voce conseguiu. A ultima chave foi encontrada.",
            "Voz: O caminho se abre em dois. O seu... e o nosso.",
            "Voz: O portal verde oferece o seu lar, a sua paz... ao custo da nossa existencia.",
            "Voz: O portal roxo vai te manter aqui, como o novo guardiao, para nos salvar.",
            "Voz: A escolha e sua, Viajante do Eter."
        },
    [this]() {
        mGame->SetGamePlayState(Game::GamePlayState::Playing);
    }
    );
        const auto &portal = new Portal(mGame);
        portal->SetPosition(Vector2(288.0f, 992.0f));

        const auto &portal2 = new Portal(mGame, 1);
        portal2->SetPosition(Vector2(416.0f, 970.0f));
    }

}

void Punk::FindHeart() {
    if (mLives < 6) {mLives++;}
    mGame->GetAudio()->PlaySound("KeyPick.wav");
}