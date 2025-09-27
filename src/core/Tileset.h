#pragma once

#include <string>
#include <SDL.h>
#include "../libs/Json.h"
#include "./Game.h"
#include <string>
using json = nlohmann::json;

class TileExtraInfo {
public:
    TileExtraInfo(
        int inId, 
        int BBOffsetX, int BBOffsetY,
        int BBWidth, int BBHeight
    ): id(inId), BBOffsetX(BBOffsetX), BBOffsetY(BBOffsetY), BBWidth(BBWidth), BBHeight(BBHeight) {};

    int id;
    int BBOffsetX, BBOffsetY;
    int BBWidth, BBHeight;
    nlohmann::json mObjectGroup;
};

class Tileset
{
public:
    Tileset(class Game* game, std::string jsonPath);
    ~Tileset();

    void Print();
    std::string GetName() const { return mName; }
    SDL_Texture* GetTexture() const { return mTexture; }

    Vector2 GetTileDims() const { return Vector2(mTileWidth, mTileHeight); }
    Vector2 GetTilesetTexturePosition(int localGID);
    Vector2 GetBBOffset(int localID);
    Vector2 GetBBSize(int localID);

private:
    int mImageWidth, mImageHeight;
    int mTileWidth, mTileHeight;
    std::string mName;
    std::map<int, TileExtraInfo> mTileExtraInfo;

    class Game* mGame;
    SDL_Texture* mTexture;

    void LoadTexture();
};
