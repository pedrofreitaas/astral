#include "DrawRectangleComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include <SDL.h>

DrawRectangleComponent::DrawRectangleComponent(Actor* owner, Vector2 size, Vector3 color, int drawOrder)
    : DrawComponent(owner, drawOrder)
    , mSize(size)
    , mColor(color)
{}

void DrawRectangleComponent::SetSize(const Vector2& size)
{
    mSize = size;
}

void DrawRectangleComponent::SetColor(const Vector3& color)
{
    mColor = color;
}

void DrawRectangleComponent::Draw(SDL_Renderer* renderer, const Vector3& modColor)
{
    SDL_SetRenderDrawColor(renderer,
        static_cast<Uint8>(mColor.x * 255),
        static_cast<Uint8>(mColor.y * 255),
        static_cast<Uint8>(mColor.z * 255),
        255);

    Vector2 pos = mOwner->GetPosition();
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    SDL_Rect rect;
    rect.x = static_cast<int>(pos.x - cameraPos.x);
    rect.y = static_cast<int>(pos.y - cameraPos.y);
    rect.w = static_cast<int>(mSize.x);
    rect.h = static_cast<int>(mSize.y);

    SDL_RenderFillRect(renderer, &rect);

    // Tamanho fixo total da barra
    const int borderWidth = 50;
    const int borderHeight = 6;

    // Define a posição da borda na mesma posição do retângulo preenchido
    // Ajusta também pelo cameraPos
    SDL_Rect borderRect;
    borderRect.x = static_cast<int>(pos.x - cameraPos.x);
    borderRect.y = static_cast<int>(pos.y - cameraPos.y);
    borderRect.w = borderWidth;
    borderRect.h = borderHeight;

    // Cor da borda (preto)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &borderRect);
}