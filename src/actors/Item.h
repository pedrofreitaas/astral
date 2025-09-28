#pragma once

#include <functional>
#include <utility>
#include <string>
#include "Actor.h"
#include "../core/Game.h"

class Item : public Actor
{
public:
    using PickHandler = std::function<void(Item &)>;

    Item(Game *game,
         const std::string &texturePath,
         PickHandler onPick = nullptr,
         int dx=0, int dy=0,
         int sizeX=Game::TILE_SIZE, int sizeY=Game::TILE_SIZE);

    void OnCollision();

private:
    bool mIsPicked;
    PickHandler mOnPickCallback;
};