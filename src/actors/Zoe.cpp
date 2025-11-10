#include "Zoe.h"
#include "Tile.h"
#include "../core/Game.h"
#include "../components/draw/DrawAnimatedComponent.h"
#include "../ui/DialogueSystem.h"
#include "../components/TimerComponent.h"

Ventania::Ventania(Game* game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed):
Actor(game)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Ventania/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Ventania/texture.json";

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Ventania::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player)-1
    );

    mDrawAnimatedComponent->AddAnimation("normal", 0, 5);
    mDrawAnimatedComponent->SetAnimation("normal");
    mDrawAnimatedComponent->SetAnimFPS(10.f);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);

    Vector2 playerFeet = playerCenter;

    SetPosition(playerCenter - mDrawAnimatedComponent->GetHalfSpriteSize());

    float directionAngle = Math::Atan2(playerMoveDir.y, playerMoveDir.x);
    float originalAngle = Math::Atan2(-1.f, 0.f);
    directionAngle -= originalAngle;

    SetRotation(directionAngle);
}

void Ventania::AnimationEndCallback(std::string animationName)
{
    if (animationName == "normal")
    {
        SetState(ActorState::Destroy);
    }
}

// 

Fireball::Fireball(
    class Game* game, Vector2 position, 
    Vector2 target, float speed
): Projectile(game, position, target, speed), mRicochetsCount(0)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Fireball/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Fireball/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        45, 28,
        10, 9 ,
        ColliderLayer::Fireball);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Fireball::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Enemy) + 1);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 28);
    mDrawAnimatedComponent->AddAnimation("dying", 29, 48);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - mDrawAnimatedComponent->GetHalfSpriteSize());
    
    mDirection = target - GetCenter();
    mDirection.Normalize();

    float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
    float originalAngle = Math::Atan2(0.f, 1.f);
    directionAngle -= originalAngle;

    SetRotation(directionAngle);
}

void Fireball::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying") {
        SetState(ActorState::Destroy);
    }
}

void Fireball::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying) {
        mDrawAnimatedComponent->SetAnimation("dying");
        mDrawAnimatedComponent->SetAnimFPS(10.f);
    }
    else if (mBehaviorState == BehaviorState::Moving) {
        mDrawAnimatedComponent->SetAnimation("flying");
        mDrawAnimatedComponent->SetAnimFPS(20.f);
    }
}

void Fireball::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        return;
    }

    if (mRicochetsCount >= MAX_RICOCHETS || other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mRicochetsCount++;

        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(Math::Sign(minOverlap), 0.f));

        reflection.Normalize();

        mDirection = reflection;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;

        SetRotation(directionAngle);
    }
}

void Fireball::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {
    if (mBehaviorState != BehaviorState::Moving) return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        return;
    }

    if (mRicochetsCount >= MAX_RICOCHETS || other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mRicochetsCount++;

        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(0.f, Math::Sign(minOverlap)));

        reflection.Normalize();

        mDirection = reflection;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;

        SetRotation(directionAngle);
    }
}

void Fireball::Kill()
{
    Projectile::Kill();

    // mColliderComponent->SetEnabled(false);
}

//

Zoe::Zoe(Game *game, const float forwardSpeed): 
    Actor(game), mForwardSpeed(forwardSpeed),
    mTryingToFireFireball(false), mIsFireballOnCooldown(false), 
    mIsVentaniaOnCooldown(false), mTryingToTriggerVentania(false)
{
    SetLives(6);
    
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 11.0f);
    mColliderComponent = new AABBColliderComponent(this, 25, 20, 15, 28,
                                                   ColliderLayer::Player);
    mTimerComponent = new TimerComponent(this);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Zoe/texture.png",
        "../assets/Sprites/Zoe/texture.json",
        std::bind(&Zoe::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawComponent->AddAnimation("idle", 0, 8);
    mDrawComponent->AddAnimation("crush", 10, 17);
    mDrawComponent->AddAnimation("blink", 19, 21);
    mDrawComponent->AddAnimation("jump", 23, 25);
    mDrawComponent->AddAnimation("run", 27, 32);
    mDrawComponent->AddAnimation("hurt", {34});

    mDrawComponent->SetAnimation("idle");
}

void Zoe::OnProcessInput(const uint8_t *state)
{
    mTryingToFireFireball = state[SDL_SCANCODE_Q];
    mTryingToTriggerVentania = mBehaviorState == BehaviorState::Jumping && state[SDL_SCANCODE_E];
    
    if (mBehaviorState == BehaviorState::Dying || 
        mBehaviorState == BehaviorState::TakingDamage || 
        mBehaviorState == BehaviorState::Jumping ||
        mBehaviorState == BehaviorState::Charging)
        return;

    if (!mRigidBodyComponent->GetOnGround())
        return;

    Vector2 movementForce = Vector2(
        state[SDL_SCANCODE_D] - state[SDL_SCANCODE_A],
        GetGame()->GetApplyGravityScene() ? 0.f : (state[SDL_SCANCODE_S] - state[SDL_SCANCODE_W])
    );

    float lengthSq = movementForce.LengthSq();
    if (lengthSq > 0.0f) {
        movementForce *= 1/(Math::Sqrt(lengthSq));
    }

    mRigidBodyComponent->ApplyForce(movementForce*mForwardSpeed);

    if (state[SDL_SCANCODE_SPACE] && mGame->GetApplyGravityScene())
    {
        float jumpForce = mRigidBodyComponent->GetVerticalForce(3); 
        mRigidBodyComponent->ApplyForce(Vector2(0.f, jumpForce));
    }
}

void Zoe::OnHandleKeyPress(const int key, const bool isPressed)
{
    if (mBehaviorState == BehaviorState::Dying)
        return;
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

            if (mTryingToTriggerVentania && !mIsVentaniaOnCooldown)
            {
                TriggerVentania();
                break;
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
            else if (
                std::abs(mRigidBodyComponent->GetVelocity().x) > 0.1f ||
                std::abs(mRigidBodyComponent->GetVelocity().y) > 0.1f && !mGame->GetApplyGravityScene()
            )
            {
                mBehaviorState = BehaviorState::Moving;
            }
            
            if (mTryingToFireFireball && !mIsFireballOnCooldown) {
                mBehaviorState = BehaviorState::Charging;
            }

            break;

        case BehaviorState::Charging:
            if (!mTryingToFireFireball) {
                mBehaviorState = BehaviorState::Idle;
            }
            break;

        case BehaviorState::TakingDamage:
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

    mColliderComponent->MaintainInMap();
    ManageAnimations();
}

void Zoe::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Idle:
        mDrawComponent->SetAnimation("idle");
        mDrawComponent->SetAnimFPS(10.0f);
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
    case BehaviorState::TakingDamage:
        mDrawComponent->SetAnimation("hurt");
        mDrawComponent->SetAnimFPS(4.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("crush");
        mDrawComponent->SetAnimFPS(10.0f);
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
    if (other->GetLayer() == ColliderLayer::Projectile)
    {
        // projeticle can dye imediately on collision, so the take damage logic is on the projectile
    }
}

void Zoe::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Projectile)
    {
        // projeticle can dye imediately on collision, so the take damage logic is on the projectile
    }

    if (other->GetLayer() == ColliderLayer::Enemy && minOverlap > 0.f)
    {
        mRigidBodyComponent->ApplyForce(
            Vector2(
                0.f, 
                mRigidBodyComponent->GetVerticalForce(3)));
    }

    if (other->GetLayer() == ColliderLayer::Enemy && minOverlap < 0.f)
    {
        TakeDamage();
    }
}

void Zoe::AnimationEndCallback(std::string animationName)
{
    if (animationName == "hurt") {
        mBehaviorState = BehaviorState::Idle;
        mInvincible = false;

        if (mLives <= 0) {
            Kill();
        }
    }

    else if (animationName == "crush") {
        FireFireball();
        mBehaviorState = BehaviorState::Idle;
    }
}

void Zoe::FireFireball()
{
    if (mIsFireballOnCooldown)
        return;
    
    auto projectile = new Fireball(
        mGame,
        GetPosition() + GetFireballOffset(),
        GetGame()->GetLogicalMousePos(),
        Zoe::FIREBALL_SPEED);

    SetFireballOnCooldown(true);
    mTimerComponent->AddTimer(Zoe::FIREBALL_COOLDOWN, [this]() {
        SetFireballOnCooldown(false);
    });
}

void Zoe::TriggerVentania()
{
    if (mIsVentaniaOnCooldown)
        return;

    Vector2 mousePos = GetGame()->GetLogicalMousePos();
    Vector2 ventaniaDir = GetCenter() - mousePos;
    ventaniaDir.Normalize();

    mRigidBodyComponent->ApplyForce(ventaniaDir * Zoe::VETANIA_SPEED);

    new Ventania(
        GetGame(),
        GetCenter(),
        ventaniaDir);

    SetVentaniaOnCooldown(true);
    mTimerComponent->AddTimer(Zoe::VETANIA_COOLDOWN, [this]() {
        SetVentaniaOnCooldown(false);
    });
}
