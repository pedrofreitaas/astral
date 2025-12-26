#include "UICursor.h"

UICursor::UICursor(Game* game,
                   const std::string &imagePath,
                   const Vector2 &initialPos,
                   const Vector2 &size,
                   const Vector3 &color)
    : UIImage(imagePath, initialPos, size, color),
      mGame(game), mSpeed(Vector2::Zero), mFowardSpeed(320.0f)
{
}

UICursor::~UICursor()
{
}

void UICursor::Update(float deltaTime)
{
    mSpeed = mGame->getNormalizedControlerPad();
    mPosition += mSpeed * mFowardSpeed * deltaTime;

    if (mSpeed.LengthSq() <= 0.f) return;

    SetAngle(GetAngle() + mFowardSpeed * deltaTime * 3.8);
    if (GetAngle() >= 360.0f)
    {
        SetAngle(GetAngle() - 360.0f);
    }
}
