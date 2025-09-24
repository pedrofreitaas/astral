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
    Tileset() : mImageWidth(0), mImageHeight(0), mTileWidth(0), mTileHeight(0), mTexture(nullptr) {};

    Tileset(class Game* game, std::string jsonPath);
    ~Tileset();

    void Print();
    std::string GetName() const { return mName; }
    SDL_Texture* GetTexture() const { return mTexture; }

    Vector2 GetTileDims() const { return Vector2(mTileWidth, mTileHeight); }
    Vector2 GetGridDims(int localGID);
    Vector2 GetBBOffset(int localID) {
        const auto& extInfo = mTileExtraInfo.find(localID);

        if (extInfo != mTileExtraInfo.end()) {
            return Vector2(
                extInfo->second.BBOffsetX, 
                extInfo->second.BBOffsetY
            );
        }
        return Vector2(0.0f,0.0f);
    }

    Vector2 GetBBSize(int localID) {
        const auto& extInfo = mTileExtraInfo.find(localID);

        if (extInfo != mTileExtraInfo.end()) {
            return Vector2(
                extInfo->second.BBWidth, 
                extInfo->second.BBHeight
            );
        }
        return Vector2(0.0f,0.0f);
    }

private:
    int mImageWidth, mImageHeight;
    int mTileWidth, mTileHeight;
    std::string mName;
    std::map<int, TileExtraInfo> mTileExtraInfo;

    class Game* mGame;
    SDL_Texture* mTexture;

    void LoadTexture();
};
