#include "Snow.h"

Snow::Snow(
    Game *game, 
    const Vector2& center,
    SnowDirection dir
) : Actor(game)
{
    // std::string textureJsonPath = "../assets/Sprites/texture.json"; no need for it here
    std::string texturePNGPath = "../assets/Sprites/Snow/texture.png";

    new DrawSpriteComponent(
        this, 
        texturePNGPath,
        36, 36, 
        static_cast<int>(DrawLayerPosition::Player)+1
    );

    SetRotation(
        dir == SnowDirection::UP ? 0.f :
        dir == SnowDirection::RIGHT ? Math::PiOver2 :
        dir == SnowDirection::DOWN ? Math::Pi :
        3 * Math::PiOver2
    );

    mPosition = center - Vector2(18,18);
}

void Snow::Unspawn()
{
    SetState(ActorState::Destroy);
}