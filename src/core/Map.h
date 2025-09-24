#pragma once

#include <string>
#include <map>
#include <string>
#include "../libs/Json.h"
#include "./Tileset.h"
#include "../actors/Tile.h"

class Game;

using json = nlohmann::json;

class Map
{
public:
    Map(class Game* game, std::string jsonPath);
    ~Map();

    void Print();

private:
    class Game* mGame;
    std::vector<class Tile*> mTiles;
    std::map<std::string, class Tileset> mTilesets;
    
    int tileWidth, tileHeight;
    int mapWidth, mapHeight;

    std::map<std::string, class Tileset> LoadAllAvailableTilesets(const std::string& baseTilesetsPath);
};
