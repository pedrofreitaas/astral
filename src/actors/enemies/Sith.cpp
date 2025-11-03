#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

SithProjectile::SithProjectile(
    class Game* game, const std::string &spriteSheetPath, 
    const std::string &spriteSheetData, Vector2 direction, 
    Vector2 position, float speed
): Projectile(game, spriteSheetPath, spriteSheetData, direction, position, speed)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f, false);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        17, 18,
        14, 12,
        ColliderLayer::Projectile);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&SithProjectile::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 2);
    mDrawAnimatedComponent->AddAnimation("dying", 3, 7, true);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position);
}

void SithProjectile::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying") {
        SetState(ActorState::Destroy);
    }
}

void SithProjectile::ManageState()
{
}

void SithProjectile::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying) {
        mDrawAnimatedComponent->SetAnimation("dying");
    }
    else if (mBehaviorState == BehaviorState::Moving) {
        mDrawAnimatedComponent->SetAnimation("flying");
    }
}

void SithProjectile::Kill()
{
    mBehaviorState = BehaviorState::Dying;
}

Sith::Sith(Game* game, float forwardSpeed, const Vector2& position)
    : Enemy(game, forwardSpeed, position), 
    mAttackCooldown(3.f), mAttackCooldownTimer(0.f)
{
    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 10.0f, false);
    mColliderComponent = new AABBColliderComponent(
        this,
        19, 19,
        21, 21,
        ColliderLayer::Enemy);

    mDrawComponent = new DrawAnimatedComponent(
        this,
        "../assets/Sprites/Enemies/Sith/texture.png",
        "../assets/Sprites/Enemies/Sith/texture.json",
        std::bind(&Sith::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Enemy) + 1);

    mAIMovementComponent = new AIMovementComponent(
        this, 
        forwardSpeed, 
        -1, 
        TypeOfMovement::Flier,
        5.f, 
        .01f);

    mDrawComponent->AddAnimation("moving", 0, 7);
    mDrawComponent->AddAnimation("damage", 8, 11);
    mDrawComponent->AddAnimation("death", 12, 15);
    mDrawComponent->AddAnimation("attack", 16, 23);
    mDrawComponent->AddAnimation("attack2", 24, 31);
    mDrawComponent->AddAnimation("charging", 32, 37);

    mDrawComponent->SetAnimation("moving");

    mBehaviorState = BehaviorState::Moving;
    SetPosition(position);
}

void Sith::FireProjectile(Vector2 &direction, float speed)
{
    if (mAttackCooldownTimer > 0.f)
        return;
    
    auto projectile = new SithProjectile(
        mGame,
        "../assets/Sprites/Enemies/Sith/Projectile/texture.png",
        "../assets/Sprites/Enemies/Sith/Projectile/texture.json",
        GetCenter(),
        direction,
        speed);

    mAttackCooldownTimer = mAttackCooldown;
}

void Sith::ManageState()
{
    auto zoe = GetGame()->GetZoe();
    if (!zoe)
        return;

    bool playerInFov = PlayerOnFov();
    float distanceSQToZoe = (zoe->GetPosition() - GetPosition()).LengthSq();

    switch (mBehaviorState)
    {
    case BehaviorState::Asleep:
        break;
    case BehaviorState::Waking:
        break;
    case BehaviorState::Idle:
        break;
    case BehaviorState::Moving:
    {
        if (mRigidBodyComponent->GetVelocity().x > 0.f)
            SetRotation(0.f);
        else if (mRigidBodyComponent->GetVelocity().x < 0.f)
            SetRotation(Math::Pi);

        if (PlayerOnSight() && mAttackCooldownTimer <= 0.f) {
            mBehaviorState = BehaviorState::Charging;
            break;
        }

        if (playerInFov && 
            mAIMovementComponent->GetMovementState() != MovementState::FollowingPath)
        {
            mAIMovementComponent->SeekPlayer();
        }
        
        break;
    }
    case BehaviorState::Charging:
        if (!PlayerOnSight()) mBehaviorState = BehaviorState::Moving;
        break;

    default:
        mBehaviorState = BehaviorState::Asleep;
        break;
    }
}

void Sith::AnimationEndCallback(std::string animationName)
{
    if (animationName == "charging") {
        Vector2 directionToZoe = GetGame()->GetZoe()->GetPosition() - GetPosition();
        directionToZoe.Normalize();
        FireProjectile(directionToZoe, 2100.f);
        mBehaviorState = BehaviorState::Moving;
    }
}

void Sith::ManageAnimations()
{
    switch (mBehaviorState)
    {
    case BehaviorState::Moving:
        mDrawComponent->SetAnimation("moving");
        mDrawComponent->SetAnimFPS(10.f);
        break;
    case BehaviorState::Charging:
        mDrawComponent->SetAnimation("charging");
        mDrawComponent->SetAnimFPS(8.f);
        break;
    default:
        mDrawComponent->SetAnimation("moving");
        break;
    }
}

void Sith::TakeDamage()
{
}

void Sith::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        mAIMovementComponent->SetFowardSpeed(-mAIMovementComponent->GetFowardSpeed());
    }
}

void Sith::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
    }
}

void Sith::OnUpdate(float deltaTime)
{
    Enemy::OnUpdate(deltaTime);

    // Update attack cooldown timer
    if (mAttackCooldownTimer > 0.f) {
        mAttackCooldownTimer -= deltaTime;
    }
}