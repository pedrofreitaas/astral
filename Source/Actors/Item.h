//
// Created by Pedro Oliveira on 30/06/23.
//

#pragma once

#include <functional>
#include <utility>
#include "Actor.h"
#include "../Components/DrawComponents/DrawComponent.h"
#include <string>

class Item : public Actor
{
public:
    using PickHandler = std::function<void(Item &)>;

    Item(Game *game,
         const std::string &texturePath,
         PickHandler onPick = nullptr,
         int dx=0, int dy=0)
        : Actor(game),
          mOnPickCallback(std::move(onPick)),
          mIsPicked(false)
    {
        new DrawSpriteComponent(this, texturePath, 
                                Game::TILE_SIZE, Game::TILE_SIZE, 
                                static_cast<int>(DrawLayerPosition::Player));
                                
        new AABBColliderComponent(this, 
                                  0, 0, 
                                  Game::TILE_SIZE, Game::TILE_SIZE,
                                  ColliderLayer::Item, true);
    }

    void OnCollision() override {
        if (mIsPicked) {
            return;
        }

        mIsPicked = true;
        SetState(ActorState::Destroy);
        if (mOnPickCallback) mOnPickCallback(*this);
    }

private:
    bool mIsPicked;
    PickHandler mOnPickCallback;
};