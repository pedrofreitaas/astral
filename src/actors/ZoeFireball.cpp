#include "ZoeFireball.h"
#include "Zoe.h"
#include "Tile.h"
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

Fireball::Fireball(
    class Game *game, Vector2 position,
    Vector2 dir, float speed, Actor *shooter) : Projectile(game, position, Vector2(0.f, 0.f), speed, shooter), mRicochetsCount(0)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Fireball/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Fireball/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);

    mColliderComponent = new AABBColliderComponent(
        this,
        45, 28,
        10, 9,
        ColliderLayer::Fireball);

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Fireball::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Enemy) - 1);
    mDrawAnimatedComponent->SetUsePivotForRotation(true);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 28);
    mDrawAnimatedComponent->AddAnimation("dying", 29, 48);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - mDrawAnimatedComponent->GetHalfSpriteSize());

    mDirection = dir;
    mDirection.Normalize();

    float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
    float originalAngle = Math::Atan2(0.f, 1.f);
    directionAngle -= originalAngle;

    SetRotation(directionAngle);
}

void Fireball::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying")
    {
        SetState(ActorState::Destroy);
    }
}

void Fireball::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying)
    {
        mDrawAnimatedComponent->SetAnimation("dying");
        mDrawAnimatedComponent->SetAnimFPS(10.f);
    }
    else if (mBehaviorState == BehaviorState::Moving)
    {
        mDrawAnimatedComponent->SetAnimation("flying");
        mDrawAnimatedComponent->SetAnimFPS(20.f);
    }
}

void Fireball::OnHorizontalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (mBehaviorState != BehaviorState::Moving)
        return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        mRicochetsCount++;

        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(Math::Sign(minOverlap), 0.f));

        reflection.Normalize();

        Vector2 zoeSpeed = other->GetOwner()->GetComponent<RigidBodyComponent>()->GetVelocity();
        zoeSpeed.Normalize();

        mDirection = reflection + zoeSpeed * 0.5f;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;
        SetRotation(directionAngle);

        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        if (mRicochetsCount >= MAX_RICOCHETS)
        {
            Kill();
            return;
        }

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

void Fireball::OnVerticalCollision(const float minOverlap, AABBColliderComponent *other)
{
    if (mBehaviorState != BehaviorState::Moving)
        return;

    if (other->GetLayer() == ColliderLayer::Player)
    {
        Vector2 reflection = Vector2::Reflect(
            mDirection,
            Vector2(Math::Sign(minOverlap), 0.f));

        reflection.Normalize();

        Vector2 zoeSpeed = other->GetOwner()->GetComponent<RigidBodyComponent>()->GetVelocity();
        zoeSpeed.Normalize();

        mDirection = reflection + zoeSpeed * 0.5f;

        float directionAngle = Math::Atan2(mDirection.y, mDirection.x);
        float originalAngle = Math::Atan2(0.f, 1.f);
        directionAngle -= originalAngle;
        SetRotation(directionAngle);

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
    mColliderComponent->SetEnabled(false);
}
