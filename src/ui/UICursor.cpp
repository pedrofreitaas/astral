#include "UICursor.h"

UICursor::UICursor(const std::string &imagePath,
                   const Vector2 &initialMousePos,
                   const Vector2 &size,
                   const Vector3 &color)
    : UIImage(imagePath, initialMousePos, size, color), mSpinningAngle(0.0f), mPreviousPos(initialMousePos)
{
}

UICursor::~UICursor()
{
}

void UICursor::Draw(SDL_Renderer *renderer, const Vector2 &mousePos)
{
    Vector2 drawPos = mousePos - mSize * 0.5f;

    float distance = (drawPos - mPreviousPos).Length();
    if (distance > 0.1f) {
        drawPos = Vector2::Lerp(mPreviousPos, drawPos, 0.25f);
        float spinningAngle = distance * 0.25f;
        mSpinningAngle += spinningAngle;
        if (mSpinningAngle >= 360.0f)
            mSpinningAngle -= 360.0f;
    }

    UIImage::Draw(renderer, drawPos, mSpinningAngle);

    mPreviousPos = drawPos;
}