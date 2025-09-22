#pragma once

#include <string>
#include <map>
#include <string>
#include "../libs/Json.h"
#include "./Tileset.h"

class Game;

using json = nlohmann::json;

struct Layer {
    int pos;
    int width;
    int height;
    std::vector<int> tileIds;
};

class Map
{
public:
    Map(class Game* game, std::string jsonPath);
    ~Map();

    void Print();
    void Draw();

private:
    std::map<std::string, class Tileset> LoadAllAvailableTilesets(const std::string& baseTilesetsPath);
    std::vector<Layer> mLayers;

    std::map<std::string, class Tileset> mTilesets;
    int tileWidth, tileHeight;
    int mapWidth, mapHeight;

    class Game* mGame;
};
