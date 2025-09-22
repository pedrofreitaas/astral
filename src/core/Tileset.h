#pragma once

#include <string>
#include <SDL.h>
#include "../libs/Json.h"
#include "./Game.h"
#include <string>
using json = nlohmann::json;

struct TileExtraInfo {
    int id;
    json mObjectGroup;
};

class Tileset
{
public:
    Tileset() : mImageWidth(0), mImageHeight(0), mTileWidth(0), mTileHeight(0), mTexture(nullptr) {};

    Tileset(class Game* game, std::string jsonPath);
    ~Tileset();

    void Print();
    std::string GetName() const { return mName; }

private:
    int mImageWidth, mImageHeight;
    int mTileWidth, mTileHeight;
    std::string mName;
    std::map<int, TileExtraInfo> mTileExtraInfo;

    class Game* mGame;
    SDL_Texture* mTexture;

    void LoadTexture();
};
