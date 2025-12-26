//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIElement.h"

UIElement::UIElement(const Vector2 &pos, const Vector2 &size, const Vector3 &color)
        :mPosition(pos)
        ,mSize(size)
        ,mColor(color)
        ,mIsEnabled(true)
{

}

const Vector2& UIElement::GetPosition() const {
    return mPosition;
}

void UIElement::SetPosition(const Vector2 &pos) {
    mPosition = pos;
}

const Vector2& UIElement::GetCenter() const {
    static Vector2 center;
    center.x = mPosition.x + mSize.x / 2.0f;
    center.y = mPosition.y + mSize.y / 2.0f;
    return center;
}