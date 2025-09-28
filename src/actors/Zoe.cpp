#include "Zoe.h"
#include "Tile.h"
#include "Projectile.h"
#include "../core/Game.h"
#include "../components/draw/DrawSpriteComponent.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"

Zoe::Zoe(Game *game, const float forwardSpeed, const float jumpSpeed): 
    Actor(game), mIsRunning(false),
    mIsDying(false), mForwardSpeed(forwardSpeed), mJumpSpeed(jumpSpeed),
    mFoundKey(false), mDeathTimer(0.0f), mLives(6), mInvincibilityTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 5.0f);
    mColliderComponent = new AABBColliderComponent(this, 14, 20, 18, 28,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(this,
                                               "../assets/Sprites/Punk/texture.png",
                                               "../assets/Sprites/Punk/texture.json",
                                               static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("idle", {0, 1, 2, 3});
    mDrawComponent->AddAnimation("run", {4, 5, 6, 7, 8, 9, 10});
    mDrawComponent->AddAnimation("dying", {11, 12, 13, 14, 15, 16});
    mDrawComponent->AddAnimation("shooting_left_arm", {3});
    mDrawComponent->AddAnimation("shooting_noarm", {17});
    mDrawComponent->AddAnimation("dash", {18, 19, 20, 21, 22, 23});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(13.0f);
}

void Zoe::OnProcessInput(const uint8_t *state)
{
    if (mIsDying)
        return;

    mIsRunning = false;

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

void Zoe::OnHandleKeyPress(const int key, const bool isPressed)
{
}

void Zoe::TakeDamage()
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

void Zoe::MaintainInbound()
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

void Zoe::OnUpdate(float deltaTime)
{
    if (mGame->GetGamePlayState() == Game::GamePlayState::Dialogue)
    {
        mDrawComponent->SetAnimation("idle");
        return;
    }

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

void Zoe::ManageAnimations()
{
    if (mIsDying)
    {
        mDrawComponent->SetAnimation("dying");
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

void Zoe::Kill()
{
    mIsDying = true;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);
    mDrawComponent->SetAnimation("dying");

    // Disable collider and rigid body
    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);

    mGame->GetAudio()->StopAllSounds();
    mGame->ResetGameScene(3.5f);
}

void Zoe::Win(AABBColliderComponent *poleCollider)
{
    mDrawComponent->SetAnimation("win");
    mGame->SetGamePlayState(Game::GamePlayState::LevelComplete);

    // Set Zoe velocity to go down
    mRigidBodyComponent->SetVelocity(Vector2::UnitY * 100.0f); // 100 pixels per second
    mRigidBodyComponent->SetApplyGravity(false);

    // Disable collider
    poleCollider->SetEnabled(false);

    // Adjust Zoe x position to grab the pole
    mPosition.Set(poleCollider->GetOwner()->GetPosition().x + Game::TILE_SIZE / 4.0f, mPosition.y);

    // Stop music
    mGame->GetAudio()->StopAllSounds();
}

void Zoe::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
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

    if (other->GetLayer() == ColliderLayer::Item)
    {
        other->GetOwner()->OnCollision();
    }
}

void Zoe::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
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

    if (other->GetLayer() == ColliderLayer::Item)
    {
        other->GetOwner()->OnCollision();
    }
}

void Zoe::FindKey()
{
    mFoundKey = true;
    mGame->GetAudio()->PlaySound("KeyPick.wav");

    if (mGame->GetGameScene() == Game::GameScene::Level1)
    {
        DialogueSystem::Get()->StartDialogue(
            {"Zoe: Uma chave! Agora, o que sera que ela abre?",
             "Zoe: Melhor eu dar uma olhada ao redor."},
            [this]() {}
        );
    }
    else
    {
        DialogueSystem::Get()->StartDialogue(
            {"Voz: Voce conseguiu. A ultima chave foi encontrada.",
             "Voz: O caminho se abre em dois. O seu... e o nosso.",
             "Voz: O portal verde oferece o seu lar, a sua paz... ao custo da nossa existencia.",
             "Voz: O portal roxo vai te manter aqui, como o novo guardiao, para nos salvar.",
             "Voz: A escolha e sua, Viajante do Eter."},
            [this](){}
        );
    }
}

void Zoe::FindHeart()
{
    if (mLives < 6)
    {
        mLives++;
    }
    mGame->GetAudio()->PlaySound("KeyPick.wav");
}