//
// Created by Lucas N. Ferreira on 03/08/23.
//

#include "Block.h"
#include "../core/Game.h"
#include "../components/draw/DrawSpriteComponent.h"
#include "../components/collider/AABBColliderComponent.h"

Block::Block(Game *game, const std::string &texturePath, const DrawLayerPosition &layer)
    : Actor(game)
{
    bool hasCollision = (layer == DrawLayerPosition::Player);
    
    new DrawSpriteComponent(this, texturePath, Game::TILE_SIZE, Game::TILE_SIZE, static_cast<int>(layer));

    if (!hasCollision) {
        return;
    }

    new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE,
                              ColliderLayer::Blocks, true);
}