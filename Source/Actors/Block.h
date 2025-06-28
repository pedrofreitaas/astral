//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "Actor.h"
#include <string>

class Block : public Actor
{
public:
    Block(Game *game, const std::string &texturePath, ColliderLayer layer, bool hasCollision=true);

    virtual void OnColision() {};
};

class Tileset
{
public:
    Tileset(const std::string &prefixPath): mPrefixPath(prefixPath) {}

    std::string GetTilePath(int tileId) const
    {
        return mPrefixPath + std::to_string(tileId) + ".png";
    }

    Block* getBlock(int tileId, bool hasCollision=true)
    {
        return new Block(mGame, GetTilePath(tileId), ColliderLayer::Blocks, hasCollision);
    }

private:
    std::string mPrefixPath;
    Game *mGame;
};

// //
// // Created by Lucas N. Ferreira on 28/09/23.
// //
//
// #pragma once
//
// #include "Actor.h"
// #include <string>
//
// class Block : public Actor
// {
// public:
//     explicit Block(Game* game, const std::string &texturePath, const bool isStatic = true);
//     virtual void OnColision() {};
//     void SetPosition(const Vector2& position)
//     {
//         Actor::SetPosition(position);
//         mOriginalPosition.Set(position.x, position.y);
//     }
//
//     void OnUpdate(float deltaTime) override;
//     void OnBump();
//     void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
//
// private:
//     const int BUMP_FORCE = 200;
//
//     Vector2 mOriginalPosition;
//
//     class AABBColliderComponent* mColliderComponent;
//     class RigidBodyComponent* mRigidBodyComponent;
// };
