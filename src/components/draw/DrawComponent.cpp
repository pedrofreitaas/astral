//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "DrawComponent.h"
#include "../../actors/Actor.h"
#include "../../core/Game.h"

DrawComponent::DrawComponent(class Actor* owner, int drawOrder)
    :Component(owner)
    ,mDrawOrder(drawOrder)
    ,mIsVisible(true)
{
    // mOwner->GetGame()->AddDrawable(this);
}

DrawComponent::~DrawComponent()
{
    // mOwner->GetGame()->RemoveDrawable(this);
}


void DrawComponent::Draw(SDL_Renderer* renderer, const Vector3 &modColor)
{

}