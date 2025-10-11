#include "./Item.h"
#include "../components/draw/DrawSpriteComponent.h"

Item::Item(Game *game,
           const std::string &texturePath,
           PickHandler onPick,
           int dx, int dy,
           int sizeX, int sizeY)
    : Actor(game),
      mOnPickCallback(std::move(onPick)),
      mIsPicked(false)
{
    new DrawSpriteComponent(this, texturePath,
                            sizeX, sizeY,
                            static_cast<int>(DrawLayerPosition::Player));

    new AABBColliderComponent(this,
                              0, 0,
                              sizeX, sizeY,
                              ColliderLayer::Item);
}

void Item::OnCollision() {
    if (mIsPicked)
    {
        return;
    }

    mIsPicked = true;
    SetState(ActorState::Destroy);
    if (mOnPickCallback)
        mOnPickCallback(*this);
}