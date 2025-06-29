#include "../Game.h"
#include "ProjectileEffect.h"
#include "../Components/DrawComponents/DrawSpriteComponent.h"
#include "../Components/DrawComponents/DrawAnimatedComponent.h"

ProjectileEffect::ProjectileEffect(Game* game, const Vector2& position, float rotation)
        :Actor(game)
        ,mDeathTimer(0.1f)
{
    Vector2 positionCentered = position + Vector2(0.0f, -14.0f);
    SetPosition(positionCentered);
    SetRotation(rotation);

    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Projectile/projectile_effect.png", 168, 28, 20);
    mDrawComponent->SetPivot(Vector2(0.0f, 0.5f));

    //TODO - mudar para animado
    // mDrawComponent = new DrawAnimatedComponent(this,
    //     "../Assets/Sprites/Projectile/projectile_effect_anim.png",
    //     "../Assets/Sprites/Projectile/projectile_effect_anim.json");

    // mDrawComponent->AddAnimation("shoot", {0, 1, 2, 3, 4, 5});
    // mDrawComponent->SetAnimation("shoot");
    // mDrawComponent->SetAnimFPS(5.0f); 
}

void ProjectileEffect::OnUpdate(float deltaTime)
{
    mDeathTimer -= deltaTime;
    if (mDeathTimer <= 0.0f) {
        SetState(ActorState::Destroy);
    }
}