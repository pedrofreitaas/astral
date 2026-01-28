#include "Zod.h"
#include "ZodProjectile.h"
#include "../../core/Game.h"
#include "../../components/draw/DrawAnimatedComponent.h"
#include "../../components/ai/AIMovementComponent.h"
#include "../Zoe.h"
#include "../Actor.h"

ZodProjectile::ZodProjectile(
    Game* game, Vector2 position, Vector2 target, float speed, Actor* zod
): Projectile(game, position, zod)
{
    const std::string spriteSheetPath = "../assets/Sprites/Enemies/Zod/Projectile/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Enemies/Zod/Projectile/texture.json";

    mRigidBodyComponent = new RigidBodyComponent(this, 1.f, 0.f, false);
    
    mColliderComponent = new AABBColliderComponent(
        this,
        3, 5,
        13, 10,
        ColliderLayer::EnemyProjectile);
    
    mColliderComponent->SetIgnoreLayers({
        ColliderLayer::Enemy,
        ColliderLayer::EnemyProjectile
    });

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        nullptr, // could use a lambda here too
        static_cast<int>(DrawLayerPosition::Player) - 10);

    mDrawAnimatedComponent->AddAnimation("flying", 0, 3);
    mDrawAnimatedComponent->SetAnimation("flying");
    
    mBehaviorState = BehaviorState::Moving;

    SetPosition(position - GetHalfSize());
    Fire(target - position, speed);
}

void ZodProjectile::ManageAnimations()
{
    if (mBehaviorState == BehaviorState::Moving)
    {
        mDrawAnimatedComponent->SetAnimation("flying");
    }
}

void ZodProjectile::OnUpdate(float deltaTime)
{
    if (mBehaviorState == BehaviorState::Dying) {
        SetState(ActorState::Destroy);
        return;
    }

    Projectile::OnUpdate(deltaTime);
    
    if (mRigidBodyComponent->GetVelocity().x > 0.0f)
    {
        SetRotation(0.0f);
    }

    else if (mRigidBodyComponent->GetVelocity().x < 0.0f)
    {
        SetRotation(Math::Pi);
    }
}
