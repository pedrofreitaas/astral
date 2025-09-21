#include "UICursor.h"

UICursor::UICursor(const std::string &imagePath,
                   const Vector2& initialMousePos,
                   const Vector2 &size,
                   const Vector3 &color)
    : UIImage(imagePath, initialMousePos, size, color)
{

}

UICursor::~UICursor()
{

}

void UICursor::Draw(SDL_Renderer* renderer, const Vector2 &mousePos)
{
    Vector2 drawPos = mousePos - mSize * 0.5f;
    
    // Draw the cursor at the mouse position
    UIImage::Draw(renderer, drawPos);
}