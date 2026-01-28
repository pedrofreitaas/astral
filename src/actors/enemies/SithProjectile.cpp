#include "Sith.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/RigidBodyComponent.h"
#include "../../components/collider/AABBColliderComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

SithProjectile::SithProjectile(
    class Game *game, Vector2 position,
    Vector2 target, Actor *sith
) : Projectile(game, position, sith)
{
    const std::string spriteSheetPath = "../assets/Sprites/Enemies/Sith/Projectile/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Enemies/Sith/Projectile/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);

    mColliderComponent = new AABBColliderComponent(
        this,
        17, 18,
        14, 12,
        ColliderLayer::EnemyProjectile);

    mColliderComponent->SetIgnoreLayers({ColliderLayer::Enemy,
                                         ColliderLayer::EnemyProjectile});

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&SithProjectile::AnimationEndCallback, this, std::placeholders::_1), // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) + 1);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 2);
    mDrawAnimatedComponent->AddAnimation("dying", 3, 7);

    mDrawAnimatedComponent->SetAnimation("flying");
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - GetHalfSize());

    Fire(target - position, mGame->GetConfig()->Get<float>("SITH.PROJECTILE_SPEED"));
}

void SithProjectile::AnimationEndCallback(std::string animationName)
{
    if (animationName == "dying")
    {
        SetState(ActorState::Destroy);
    }
}

void SithProjectile::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Dying)
    {
        mDrawAnimatedComponent->SetAnimation("dying");
    }
    else if (mBehaviorState == BehaviorState::Moving)
    {
        mDrawAnimatedComponent->SetAnimation("flying");
    }
}