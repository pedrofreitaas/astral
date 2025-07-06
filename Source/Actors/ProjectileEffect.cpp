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

    mDrawComponent = new DrawSpriteComponent(this, "../Assets/Sprites/Projectile/projectile_effect.png", 168, 28, 999);
    mDrawComponent->SetPivot(Vector2(0.0f, 0.5f));
}

void ProjectileEffect::OnUpdate(float deltaTime)
{
    mDeathTimer -= deltaTime;
    if (mDeathTimer <= 0.0f) {
        SetState(ActorState::Destroy);
    }
}