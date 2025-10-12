#include "Zoe.h"
#include "Tile.h"
#include "Projectile.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"

Zoe::Zoe(Game *game, const float forwardSpeed): 
    Actor(game), mForwardSpeed(forwardSpeed),
    mDeathTimer(DEATH_TIMER), mLives(6), mInvincibilityTimer(0.0f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 10.0f);
    mColliderComponent = new AABBColliderComponent(this, 25, 20, 15, 28,
                                                   ColliderLayer::Player);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Zoe/texture.png",
        "../assets/Sprites/Zoe/texture.json",
        nullptr,
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("idle", 0, 8);
    mDrawComponent->AddAnimation("crush", 10, 17);
    mDrawComponent->AddAnimation("blink", 19, 21);
    mDrawComponent->AddAnimation("jump", 23, 25);
    mDrawComponent->AddAnimation("run", 27, 32);
    mDrawComponent->AddAnimation("hurt", {34});

    mDrawComponent->SetAnimation("idle");
    mDrawComponent->SetAnimFPS(10.0f);
}

void Zoe::OnProcessInput(const uint8_t *state)
{
    if (mBehaviorState == BehaviorState::Dying)
        return;

    mBehaviorState = BehaviorState::Idle;

    if (!mRigidBodyComponent->GetOnGround())
        return;

    if (state[SDL_SCANCODE_D])
    {
        mRigidBodyComponent->ApplyForce(Vector2(mForwardSpeed, 0.0f));
    }

    if (state[SDL_SCANCODE_A])
    {
        mRigidBodyComponent->ApplyForce(Vector2(-mForwardSpeed, 0.0f));
    }

    if (state[SDL_SCANCODE_SPACE])
    {
        float jumpForce = mRigidBodyComponent->GetVerticalForce(5); 
        SDL_Log("%f", jumpForce);
        mRigidBodyComponent->ApplyForce(Vector2(0.f, jumpForce));
    }
}

void Zoe::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mBehaviorState == BehaviorState::Dying)
        return;
}

void Zoe::TakeDamage()
{
    if (mBehaviorState == BehaviorState::Dying)
        return;

    if (mInvincibilityTimer > 0.0f)
        return;

    mLives--;
    mInvincibilityTimer = 0.25f;

    if (mLives <= 0)
    {
        mBehaviorState = BehaviorState::Dying;
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

void Zoe::ManageState()
{
    switch (mBehaviorState)
    {
        case BehaviorState::Dying:
            break;
        
        case BehaviorState::Jumping:
            if (mRigidBodyComponent->GetOnGround())
            {
                mBehaviorState = BehaviorState::Idle;
            }
            break;

        case BehaviorState::Moving:
            if (mRigidBodyComponent->GetOnGround() && std::abs(mRigidBodyComponent->GetVelocity().x) < 0.1f)
            {
                mBehaviorState = BehaviorState::Idle;
            }
            if (!mRigidBodyComponent->GetOnGround())
            {
                mBehaviorState = BehaviorState::Jumping;
            }    

            break;

        case BehaviorState::Idle:
            if (!mRigidBodyComponent->GetOnGround())
            {
                mBehaviorState = BehaviorState::Jumping;
            }
            else if (std::abs(mRigidBodyComponent->GetVelocity().x) > 0.1f)
            {
                mBehaviorState = BehaviorState::Moving;
            }
            break;

        default:
            mBehaviorState = BehaviorState::Idle;
            break;
    }
}

void Zoe::OnUpdate(float deltaTime)
{
    if (mGame->GetGamePlayState() == Game::GamePlayState::Dialogue)
    {
        mDrawComponent->SetAnimation("idle");
        return;
    }

    ManageState();

    if (mRigidBodyComponent->GetVelocity().x > 0.0f)
    {
        SetRotation(0.0f);
    }

    else if (mRigidBodyComponent->GetVelocity().x < 0.0f)
    {
        SetRotation(Math::Pi);
    }

    MaintainInbound();
    ManageAnimations();

    if (mBehaviorState == BehaviorState::Dying)
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
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        break;
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("run");
        break;
    case BehaviorState::Jumping:
        mDrawComponent->SetAnimation("jump");
        break;
    case BehaviorState::Dying:
        mDrawComponent->SetAnimation("dying");
        break;
    default:
        break;
    }
}

void Zoe::Kill()
{
    mBehaviorState = BehaviorState::Dying;
    mGame->SetGamePlayState(Game::GamePlayState::GameOver);

    mRigidBodyComponent->SetEnabled(false);
    mColliderComponent->SetEnabled(false);
}

void Zoe::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
}

void Zoe::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
}

void Zoe::FindHeart()
{
    if (mLives < 6)
    {
        mLives++;
    }
    mGame->GetAudio()->PlaySound("KeyPick.wav");
}