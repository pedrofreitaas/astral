#pragma once

#include <string>
#include <SDL.h>
#include "../libs/Json.h"
#include <string>
using json = nlohmann::json;

struct Tile {
    int id;
    json mObjectGroup;
};

class Tileset
{
public:
    Tileset(std::string jsonPath);
    ~Tileset();

    void print();
    std::string GetName() const { return mName; }

private:
    int mImageWidth, mImageHeight;
    int mTileWidth, mTileHeight;
    std::string mName;
    std::vector<Tile> mTiles;
};
