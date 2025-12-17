#include "Ventania.h"

Ventania::Ventania(Game *game, Vector2 playerCenter, Vector2 playerMoveDir, float forwardSpeed) : Actor(game)
{
    const std::string spriteSheetPath = "../assets/Sprites/Zoe/Ventania/texture.png";
    const std::string spriteSheetData = "../assets/Sprites/Zoe/Ventania/texture.json";

    mDrawAnimatedComponent = new DrawAnimatedComponent(
        this,
        spriteSheetPath,
        spriteSheetData,
        std::bind(&Ventania::AnimationEndCallback, this, std::placeholders::_1),
        static_cast<int>(DrawLayerPosition::Player) - 1);

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